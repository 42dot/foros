/*
 * Copyright (c) 2021 42dot All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_TEMPLATE_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_TEMPLATE_HPP_

#include <rclcpp/create_client.hpp>
#include <rclcpp/create_publisher.hpp>
#include <rclcpp/exceptions.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "akit/failover/foros/cluster_node.hpp"
#include "akit/failover/foros/common.hpp"

namespace akit {
namespace failover {
namespace foros {

// Implementation of templates of ClusterNodes

template <typename MessageT, typename AllocatorT>
std::shared_ptr<ClusterNodePublisher<MessageT, AllocatorT>>
ClusterNode::create_publisher(
    const std::string &topic_name, const rclcpp::QoS &qos,
    const rclcpp::PublisherOptionsWithAllocator<AllocatorT> &options) {
  auto pub =
      rclcpp::create_publisher<MessageT, AllocatorT,
                               ClusterNodePublisher<MessageT, AllocatorT>>(
          *this, topic_name, qos, options);
  pub->set_node_lifecycle_interface(this);
  return pub;
}

template <typename MessageT, typename CallbackT, typename AllocatorT,
          typename CallbackMessageT, typename SubscriptionT,
          typename MessageMemoryStrategyT>
std::shared_ptr<SubscriptionT> ClusterNode::create_subscription(
    const std::string &topic_name, const rclcpp::QoS &qos, CallbackT &&callback,
    const rclcpp::SubscriptionOptionsWithAllocator<AllocatorT> &options,
    typename MessageMemoryStrategyT::SharedPtr msg_mem_strat) {
  return rclcpp::create_subscription<MessageT>(
      *this, topic_name, qos, std::forward<CallbackT>(callback), options,
      msg_mem_strat);
}

template <typename DurationRepT, typename DurationT, typename CallbackT>
typename rclcpp::WallTimer<CallbackT>::SharedPtr ClusterNode::create_wall_timer(
    std::chrono::duration<DurationRepT, DurationT> period, CallbackT callback,
    rclcpp::CallbackGroup::SharedPtr group) {
  return rclcpp::create_wall_timer(period, std::move(callback), group,
                                   node_base_.get(), node_timers_.get());
}

template <typename ServiceT>
typename rclcpp::Client<ServiceT>::SharedPtr ClusterNode::create_client(
    const std::string &service_name, const rmw_qos_profile_t &qos_profile,
    rclcpp::CallbackGroup::SharedPtr group) {
  return rclcpp::create_client<ServiceT>(node_base_, node_graph_,
                                         node_services_, service_name,
                                         qos_profile, group);
}

template <typename ServiceT, typename CallbackT>
typename ClusterNodeService<ServiceT>::SharedPtr ClusterNode::create_service(
    const std::string &service_name, CallbackT &&callback,
    const rmw_qos_profile_t &qos_profile,
    rclcpp::CallbackGroup::SharedPtr group) {
  rclcpp::AnyServiceCallback<ServiceT> any_service_callback;
  any_service_callback.set(std::forward<CallbackT>(callback));

  rcl_service_options_t service_options = rcl_service_get_default_options();
  service_options.qos = qos_profile;

  auto service = ClusterNodeService<ServiceT>::make_shared(
      node_base_->get_shared_rcl_node_handle(), service_name,
      any_service_callback, service_options);
  auto serv_base_ptr = std::dynamic_pointer_cast<rclcpp::ServiceBase>(service);
  node_services_->add_service(serv_base_ptr, group);
  service->set_node_lifecycle_interface(this);
  return service;
}

template <typename ParameterT>
auto ClusterNode::declare_parameter(
    const std::string &name, const ParameterT &default_value,
    const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor,
    bool ignore_override) {
  try {
    return declare_parameter(name, rclcpp::ParameterValue(default_value),
                             parameter_descriptor, ignore_override)
        .get<ParameterT>();
  } catch (const rclcpp::ParameterTypeException &ex) {
    throw rclcpp::exceptions::InvalidParameterTypeException(name, ex.what());
  }
}

template <typename ParameterT>
auto ClusterNode::declare_parameter(
    const std::string &name,
    const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor,
    bool ignore_override) {
  // get advantage of parameter value template magic
  // to get
  // the correct rclcpp::ParameterType from ParameterT
  rclcpp::ParameterValue value{ParameterT{}};
  return this
      ->declare_parameter(name, value.get_type(), parameter_descriptor,
                          ignore_override)
      .get<ParameterT>();
}

template <typename ParameterT>
std::vector<ParameterT> ClusterNode::declare_parameters(
    const std::string &parameter_namespace,
    const std::map<std::string, ParameterT> &parameters,
    bool ignore_overrides) {
  std::vector<ParameterT> result;
  std::string normalized_namespace =
      parameter_namespace.empty() ? "" : (parameter_namespace + ".");
  std::transform(
      parameters.begin(), parameters.end(), std::back_inserter(result),
      [this, &normalized_namespace, ignore_overrides](auto element) {
        return this->declare_parameter(
            normalized_namespace + element.first, element.second,
            rcl_interfaces::msg::ParameterDescriptor(), ignore_overrides);
      });
  return result;
}

template <typename ParameterT>
std::vector<ParameterT> ClusterNode::declare_parameters(
    const std::string &parameter_namespace,
    const std::map<
        std::string,
        std::pair<ParameterT, rcl_interfaces::msg::ParameterDescriptor>>
        &parameters,
    bool ignore_overrides) {
  std::vector<ParameterT> result;
  std::string normalized_namespace =
      parameter_namespace.empty() ? "" : (parameter_namespace + ".");
  std::transform(
      parameters.begin(), parameters.end(), std::back_inserter(result),
      [this, &normalized_namespace, ignore_overrides](auto element) {
        return static_cast<ParameterT>(this->declare_parameter(
            normalized_namespace + element.first, element.second.first,
            element.second.second, ignore_overrides));
      });
  return result;
}

template <typename ParameterT>
bool ClusterNode::get_parameter(const std::string &name,
                                ParameterT &parameter) const {
  rclcpp::Parameter parameter_variant;

  bool result = get_parameter(name, parameter_variant);
  if (result) {
    parameter =
        static_cast<ParameterT>(parameter_variant.get_value<ParameterT>());
  }

  return result;
}

template <typename ParameterT>
bool ClusterNode::get_parameter_or(const std::string &name,
                                   ParameterT &parameter,
                                   const ParameterT &alternative_value) const {
  bool got_parameter = get_parameter(name, parameter);
  if (!got_parameter) {
    parameter = alternative_value;
  }
  return got_parameter;
}

// this is a partially-specialized version of get_parameter above,
// where our concrete type for ParameterT is std::map, but the to-be-determined
// type is the value in the map.
template <typename ParameterT>
bool ClusterNode::get_parameters(
    const std::string &prefix,
    std::map<std::string, ParameterT> &values) const {
  std::map<std::string, rclcpp::Parameter> params;
  bool result = node_parameters_->get_parameters_by_prefix(prefix, params);
  if (result) {
    for (const auto &param : params) {
      values[param.first] =
          static_cast<ParameterT>(param.second.get_value<ParameterT>());
    }
  }

  return result;
}

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_TEMPLATE_HPP_
