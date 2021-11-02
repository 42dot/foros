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

#ifndef AKIT_FAILSAFE_FSROS_CLUSTER_NODE_HPP_
#define AKIT_FAILSAFE_FSROS_CLUSTER_NODE_HPP_

#include <rclcpp/callback_group.hpp>
#include <rclcpp/create_client.hpp>
#include <rclcpp/create_publisher.hpp>
#include <rclcpp/create_service.hpp>
#include <rclcpp/create_subscription.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_timers.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/node_options.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "akit/failsafe/fsros/cluster_node_interface.hpp"
#include "akit/failsafe/fsros/cluster_node_options.hpp"
#include "akit/failsafe/fsros/cluster_node_publisher.hpp"
#include "akit/failsafe/fsros/cluster_node_service.hpp"
#include "akit/failsafe/fsros/common.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

class ClusterNodeImpl;

/// ClusterNode for creating a clustered node.
/**
 * has cluster node interfaces.
 */
class ClusterNode : public ClusterNodeInterface {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(ClusterNode)

  /// Create a new cluster node with the specified name.
  /**
   * \param[in] node_id ID of the node
   * \param[in] cluster_name Cluster name of the node. (name space)
   * \param[in] cluster_node_ids IDs of nodes in the cluster
   * \param[in] options Additional options to control creation of the node.
   */
  CLUSTER_NODE_PUBLIC
  explicit ClusterNode(
      const uint32_t node_id, const std::string &cluster_name,
      const std::vector<uint32_t> &cluster_node_ids,
      const ClusterNodeOptions &options = ClusterNodeOptions());

  CLUSTER_NODE_PUBLIC
  virtual ~ClusterNode();

  /// Get the name of the node.
  /**
   * \return The name of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_name() const;

  /// Get the namespace of the node
  /**
   * \return The namespace of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_namespace() const;

  /// Get the logger of the node.
  /**
   * \return The logger of the node.
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Logger get_logger() const;

  /// Create and return a callback group.
  /**
   * \param[in] group_type callback group type to create by this method.
   * \param[in] automatically_add_to_executor_with_node A boolean that
   *   determines whether a callback group is automatically added to an executor
   *   with the node with which it is associated.
   * \return a callback group
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::CallbackGroup::SharedPtr create_callback_group(
      rclcpp::CallbackGroupType group_type,
      bool automatically_add_to_executor_with_node = true);

  /// Return the list of callback groups in the node.
  /**
   * \return list of callback groups in the node.
   */
  CLUSTER_NODE_PUBLIC
  const std::vector<rclcpp::CallbackGroup::WeakPtr> &get_callback_groups()
      const;

  /// Create and return a Publisher.
  /**
   * \param[in] topic_name The topic for this publisher to publish on.
   * \param[in] qos The Quality of Service settings for this publisher.
   * \param[in] options The publisher options for this publisher.
   * \return Shared pointer to the created cluster node publisher.
   */
  template <typename MessageT, typename AllocatorT = std::allocator<void>>
  std::shared_ptr<ClusterNodePublisher<MessageT, AllocatorT>> create_publisher(
      const std::string &topic_name, const rclcpp::QoS &qos,
      const rclcpp::PublisherOptionsWithAllocator<AllocatorT> &options =
          (rclcpp::PublisherOptionsWithAllocator<AllocatorT>())) {
    auto pub =
        rclcpp::create_publisher<MessageT, AllocatorT,
                                 ClusterNodePublisher<MessageT, AllocatorT>>(
            *this, topic_name, qos, options);
    pub->set_node_interface(this);
    return pub;
  }

  /// Create and return a Subscription.
  /**
   * \param[in] topic_name The topic to subscribe on.
   * \param[in] callback The user-defined callback function.
   * \param[in] qos The quality of service for this subscription.
   * \param[in] options The subscription options for this subscription.
   * \param[in] msg_mem_strat The message memory strategy to use for allocating
   * messages. \return Shared pointer to the created subscription.
   */
  template <typename MessageT, typename CallbackT,
            typename AllocatorT = std::allocator<void>,
            typename CallbackMessageT = typename rclcpp::subscription_traits::
                has_message_type<CallbackT>::type,
            typename SubscriptionT = rclcpp::Subscription<MessageT, AllocatorT>,
            typename MessageMemoryStrategyT = rclcpp::message_memory_strategy::
                MessageMemoryStrategy<CallbackMessageT, AllocatorT>>
  std::shared_ptr<SubscriptionT> create_subscription(
      const std::string &topic_name, const rclcpp::QoS &qos,
      CallbackT &&callback,
      const rclcpp::SubscriptionOptionsWithAllocator<AllocatorT> &options =
          rclcpp::SubscriptionOptionsWithAllocator<AllocatorT>(),
      typename MessageMemoryStrategyT::SharedPtr msg_mem_strat =
          (MessageMemoryStrategyT::create_default())) {
    {
      return rclcpp::create_subscription<MessageT>(
          *this, topic_name, qos, std::forward<CallbackT>(callback), options,
          msg_mem_strat);
    }
  }

  /// Create a timer.
  /**
   * \param[in] period Time interval between triggers of the callback.
   * \param[in] callback User-defined callback function.
   * \param[in] group Callback group to execute this timer's callback in.
   */
  template <typename DurationRepT = int64_t, typename DurationT = std::milli,
            typename CallbackT>
  typename rclcpp::WallTimer<CallbackT>::SharedPtr create_wall_timer(
      std::chrono::duration<DurationRepT, DurationT> period, CallbackT callback,
      rclcpp::CallbackGroup::SharedPtr group = nullptr) {
    auto timer = rclcpp::WallTimer<CallbackT>::make_shared(
        std::chrono::duration_cast<std::chrono::nanoseconds>(period),
        std::move(callback), this->node_base_->get_context());
    node_timers_->add_timer(timer, group);
    return timer;
  }

  /// Create and return a Client.
  /**
   * \sa rclcpp::Node::create_client
   */
  template <typename ServiceT>
  typename rclcpp::Client<ServiceT>::SharedPtr create_client(
      const std::string &service_name,
      const rmw_qos_profile_t &qos_profile = rmw_qos_profile_services_default,
      rclcpp::CallbackGroup::SharedPtr group = nullptr) {
    rcl_client_options_t options = rcl_client_get_default_options();
    options.qos = qos_profile;

    using rclcpp::Client;
    using rclcpp::ClientBase;

    auto cli = Client<ServiceT>::make_shared(node_base_.get(), node_graph_,
                                             service_name, options);

    auto cli_base_ptr = std::dynamic_pointer_cast<ClientBase>(cli);
    node_services_->add_client(cli_base_ptr, group);
    return cli;
  }

  /// Create and return a Service.
  /**
   * \sa rclcpp::Node::create_service
   */
  template <typename ServiceT, typename CallbackT>
  typename ClusterNodeService<ServiceT>::SharedPtr create_service(
      const std::string &service_name, CallbackT &&callback,
      const rmw_qos_profile_t &qos_profile = rmw_qos_profile_services_default,
      rclcpp::CallbackGroup::SharedPtr group = nullptr) {
    rclcpp::AnyServiceCallback<ServiceT> any_service_callback;
    any_service_callback.set(std::forward<CallbackT>(callback));

    rcl_service_options_t service_options = rcl_service_get_default_options();
    service_options.qos = qos_profile;

    auto service = ClusterNodeService<ServiceT>::make_shared(
        node_base_->get_shared_rcl_node_handle(), service_name,
        any_service_callback, service_options);
    auto serv_base_ptr =
        std::dynamic_pointer_cast<rclcpp::ServiceBase>(service);
    node_services_->add_service(serv_base_ptr, group);
    service->set_node_interface(this);
    return service;
  }

  /// Return the Node's internal NodeBaseInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_base_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr
  get_node_base_interface();

  /// Return the Node's internal NodeGraphInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_graph_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr
  get_node_graph_interface();

  /// Return the Node's internal NodeTimersInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_timers_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
  get_node_timers_interface();

  /// Return the Node's internal NodeTopicsInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_topics_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
  get_node_topics_interface();

  /// Return the Node's internal NodeServicesInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_services_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr
  get_node_services_interface();

  /// Return the Node's internal NodeClockInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_clock_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr
  get_node_clock_interface();

  /// Callback function for activate transition
  CLUSTER_NODE_PUBLIC
  void on_activated() override;

  /// Callback function for deactivate transition
  CLUSTER_NODE_PUBLIC
  void on_deactivated() override;

  /// Callback function for standby transition
  CLUSTER_NODE_PUBLIC
  void on_standby() override;

  /// Check whether the node is activated or not
  /**
   * \return true if the node is activated, false if not
   */
  CLUSTER_NODE_PUBLIC
  bool is_activated() final;

 private:
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph_;
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging_;
  rclcpp::node_interfaces::NodeTimers::SharedPtr node_timers_;
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics_;
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services_;
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock_;

  std::unique_ptr<ClusterNodeImpl> impl_;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_CLUSTER_NODE_HPP_
