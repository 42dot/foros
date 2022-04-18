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

#include "akit/failover/foros/cluster_node.hpp"

#include <rclcpp/node_interfaces/node_base.hpp>
#include <rclcpp/node_interfaces/node_clock.hpp>
#include <rclcpp/node_interfaces/node_graph.hpp>
#include <rclcpp/node_interfaces/node_logging.hpp>
#include <rclcpp/node_interfaces/node_parameters.hpp>
#include <rclcpp/node_interfaces/node_services.hpp>
#include <rclcpp/node_interfaces/node_time_source.hpp>
#include <rclcpp/node_interfaces/node_topics.hpp>
#include <rclcpp/node_interfaces/node_waitables.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node_options.hpp"
#include "cluster_node_impl.hpp"
#include "common/node_util.hpp"

namespace akit {
namespace failover {
namespace foros {

ClusterNode::ClusterNode(const std::string &cluster_name,
                         const uint32_t node_id,
                         const std::vector<uint32_t> &cluster_node_ids,
                         const ClusterNodeOptions &options)
    : ClusterNode(cluster_name, node_id, cluster_node_ids, "", options) {}

ClusterNode::ClusterNode(const std::string &cluster_name,
                         const uint32_t node_id,
                         const std::vector<uint32_t> &cluster_node_ids,
                         const std::string &node_namespace,
                         const ClusterNodeOptions &options)
    : node_base_(new rclcpp::node_interfaces::NodeBase(
          NodeUtil::get_node_name(cluster_name, node_id), node_namespace,
          options.context(), *(options.get_rcl_node_options()),
          options.use_intra_process_comms(),
          options.enable_topic_statistics())),
      node_graph_(new rclcpp::node_interfaces::NodeGraph(node_base_.get())),
      node_logging_(new rclcpp::node_interfaces::NodeLogging(node_base_.get())),
      node_timers_(new rclcpp::node_interfaces::NodeTimers(node_base_.get())),
      node_topics_(new rclcpp::node_interfaces::NodeTopics(node_base_.get(),
                                                           node_timers_.get())),
      node_services_(
          new rclcpp::node_interfaces::NodeServices(node_base_.get())),
      node_clock_(new rclcpp::node_interfaces::NodeClock(
          node_base_, node_topics_, node_graph_, node_services_,
          node_logging_)),
      node_parameters_(new rclcpp::node_interfaces::NodeParameters(
          node_base_, node_logging_, node_topics_, node_services_, node_clock_,
          options.parameter_overrides(), options.start_parameter_services(),
          options.start_parameter_event_publisher(),
          options.parameter_event_qos(),
          options.parameter_event_publisher_options(),
          options.allow_undeclared_parameters(),
          options.automatically_declare_parameters_from_overrides())),
      node_time_source_(new rclcpp::node_interfaces::NodeTimeSource(
          node_base_, node_topics_, node_graph_, node_services_, node_logging_,
          node_clock_, node_parameters_, options.clock_qos(),
          options.use_clock_thread())),
      node_waitables_(
          new rclcpp::node_interfaces::NodeWaitables(node_base_.get())),
      impl_(std::make_unique<ClusterNodeImpl>(
          cluster_name, node_id, cluster_node_ids, node_base_, node_graph_,
          node_logging_, node_services_, node_topics_, node_timers_,
          node_clock_, options)) {}

ClusterNode::~ClusterNode() {
  // release sub-interfaces in an order that allows them to consult with
  // node_base during tear-down
  node_waitables_.reset();
  node_time_source_.reset();
  node_parameters_.reset();
  node_clock_.reset();
  node_services_.reset();
  node_topics_.reset();
  node_timers_.reset();
  node_logging_.reset();
  node_graph_.reset();
}

const char *ClusterNode::get_name() const { return node_base_->get_name(); }

const char *ClusterNode::get_namespace() const {
  return node_base_->get_namespace();
}

const char *ClusterNode::get_fully_qualified_name() const {
  return node_base_->get_fully_qualified_name();
}

rclcpp::Logger ClusterNode::get_logger() const {
  return node_logging_->get_logger();
}

rclcpp::CallbackGroup::SharedPtr ClusterNode::create_callback_group(
    rclcpp::CallbackGroupType group_type,
    bool automatically_add_to_executor_with_node) {
  return node_base_->create_callback_group(
      group_type, automatically_add_to_executor_with_node);
}

const std::vector<rclcpp::CallbackGroup::WeakPtr>
    &ClusterNode::get_callback_groups() const {
  return node_base_->get_callback_groups();
}

const rclcpp::ParameterValue &ClusterNode::declare_parameter(
    const std::string &name, const rclcpp::ParameterValue &default_value,
    const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor,
    bool ignore_override) {
  return node_parameters_->declare_parameter(
      name, default_value, parameter_descriptor, ignore_override);
}

const rclcpp::ParameterValue &ClusterNode::declare_parameter(
    const std::string &name, rclcpp::ParameterType type,
    const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor,
    bool ignore_override) {
  return node_parameters_->declare_parameter(name, type, parameter_descriptor,
                                             ignore_override);
}

void ClusterNode::undeclare_parameter(const std::string &name) {
  node_parameters_->undeclare_parameter(name);
}

bool ClusterNode::has_parameter(const std::string &name) const {
  return node_parameters_->has_parameter(name);
}

rcl_interfaces::msg::SetParametersResult ClusterNode::set_parameter(
    const rclcpp::Parameter &parameter) {
  return set_parameters_atomically({parameter});
}

std::vector<rcl_interfaces::msg::SetParametersResult>
ClusterNode::set_parameters(const std::vector<rclcpp::Parameter> &parameters) {
  return node_parameters_->set_parameters(parameters);
}

rcl_interfaces::msg::SetParametersResult ClusterNode::set_parameters_atomically(
    const std::vector<rclcpp::Parameter> &parameters) {
  return node_parameters_->set_parameters_atomically(parameters);
}

rclcpp::Parameter ClusterNode::get_parameter(const std::string &name) const {
  return node_parameters_->get_parameter(name);
}

bool ClusterNode::get_parameter(const std::string &name,
                                rclcpp::Parameter &parameter) const {
  return node_parameters_->get_parameter(name, parameter);
}

std::vector<rclcpp::Parameter> ClusterNode::get_parameters(
    const std::vector<std::string> &names) const {
  return node_parameters_->get_parameters(names);
}

rcl_interfaces::msg::ParameterDescriptor ClusterNode::describe_parameter(
    const std::string &name) const {
  auto result = node_parameters_->describe_parameters({name});
  if (0 == result.size()) {
    throw rclcpp::exceptions::ParameterNotDeclaredException(name);
  }
  if (result.size() > 1) {
    throw std::runtime_error(
        "number of described parameters unexpectedly more than one");
  }
  return result.front();
}

std::vector<rcl_interfaces::msg::ParameterDescriptor>
ClusterNode::describe_parameters(const std::vector<std::string> &names) const {
  return node_parameters_->describe_parameters(names);
}

std::vector<uint8_t> ClusterNode::get_parameter_types(
    const std::vector<std::string> &names) const {
  return node_parameters_->get_parameter_types(names);
}

rcl_interfaces::msg::ListParametersResult ClusterNode::list_parameters(
    const std::vector<std::string> &prefixes, uint64_t depth) const {
  return node_parameters_->list_parameters(prefixes, depth);
}

ClusterNode::OnSetParametersCallbackHandle::SharedPtr
ClusterNode::add_on_set_parameters_callback(
    OnParametersSetCallbackType callback) {
  return node_parameters_->add_on_set_parameters_callback(callback);
}

void ClusterNode::remove_on_set_parameters_callback(
    const OnSetParametersCallbackHandle *const callback) {
  return node_parameters_->remove_on_set_parameters_callback(callback);
}

std::vector<std::string> ClusterNode::get_node_names() const {
  return node_graph_->get_node_names();
}

std::map<std::string, std::vector<std::string>>
ClusterNode::get_topic_names_and_types() const {
  return node_graph_->get_topic_names_and_types();
}

std::map<std::string, std::vector<std::string>>
ClusterNode::get_service_names_and_types() const {
  return node_graph_->get_service_names_and_types();
}

std::map<std::string, std::vector<std::string>>
ClusterNode::get_service_names_and_types_by_node(
    const std::string &node_name, const std::string &node_namespace) const {
  return node_graph_->get_service_names_and_types_by_node(node_name,
                                                          node_namespace);
}

size_t ClusterNode::count_publishers(const std::string &topic_name) const {
  return node_graph_->count_publishers(topic_name);
}

size_t ClusterNode::count_subscribers(const std::string &topic_name) const {
  return node_graph_->count_subscribers(topic_name);
}

std::vector<rclcpp::TopicEndpointInfo>
ClusterNode::get_publishers_info_by_topic(const std::string &topic_name,
                                          bool no_mangle) const {
  return node_graph_->get_publishers_info_by_topic(topic_name, no_mangle);
}

std::vector<rclcpp::TopicEndpointInfo>
ClusterNode::get_subscriptions_info_by_topic(const std::string &topic_name,
                                             bool no_mangle) const {
  return node_graph_->get_subscriptions_info_by_topic(topic_name, no_mangle);
}

rclcpp::Event::SharedPtr ClusterNode::get_graph_event() {
  return node_graph_->get_graph_event();
}

void ClusterNode::wait_for_graph_change(rclcpp::Event::SharedPtr event,
                                        std::chrono::nanoseconds timeout) {
  node_graph_->wait_for_graph_change(event, timeout);
}

rclcpp::Clock::SharedPtr ClusterNode::get_clock() {
  return node_clock_->get_clock();
}

rclcpp::Clock::ConstSharedPtr ClusterNode::get_clock() const {
  return node_clock_->get_clock();
}

rclcpp::Time ClusterNode::now() const {
  return node_clock_->get_clock()->now();
}

rclcpp::node_interfaces::NodeBaseInterface::SharedPtr
ClusterNode::get_node_base_interface() {
  return node_base_;
}

rclcpp::node_interfaces::NodeClockInterface::SharedPtr
ClusterNode::get_node_clock_interface() {
  return node_clock_;
}

rclcpp::node_interfaces::NodeGraphInterface::SharedPtr
ClusterNode::get_node_graph_interface() {
  return node_graph_;
}

rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
ClusterNode::get_node_logging_interface() {
  return node_logging_;
}

rclcpp::node_interfaces::NodeTimeSourceInterface::SharedPtr
ClusterNode::get_node_time_source_interface() {
  return node_time_source_;
}

rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
ClusterNode::get_node_timers_interface() {
  return node_timers_;
}

rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
ClusterNode::get_node_topics_interface() {
  return node_topics_;
}

rclcpp::node_interfaces::NodeServicesInterface::SharedPtr
ClusterNode::get_node_services_interface() {
  return node_services_;
}

rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
ClusterNode::get_node_parameters_interface() {
  return node_parameters_;
}

rclcpp::node_interfaces::NodeWaitablesInterface::SharedPtr
ClusterNode::get_node_waitables_interface() {
  return node_waitables_;
}

bool ClusterNode::is_activated() { return impl_->is_activated(); }

void ClusterNode::register_on_activated(std::function<void()> callback) {
  impl_->register_on_activated(callback);
}

void ClusterNode::register_on_deactivated(std::function<void()> callback) {
  impl_->register_on_deactivated(callback);
}

void ClusterNode::register_on_standby(std::function<void()> callback) {
  impl_->register_on_standby(callback);
}

CommandCommitResponseSharedFuture ClusterNode::commit_command(
    Command::SharedPtr command, CommandCommitResponseCallback callback) {
  return impl_->commit_command(command, callback);
}

uint64_t ClusterNode::get_commands_size() { return impl_->get_commands_size(); }

Command::SharedPtr ClusterNode::get_command(uint64_t id) {
  return impl_->get_command(id);
}

void ClusterNode::register_on_committed(
    std::function<void(const uint64_t, Command::SharedPtr)> callback) {
  impl_->register_on_committed(callback);
}

void ClusterNode::register_on_reverted(
    std::function<void(const uint64_t)> callback) {
  impl_->register_on_reverted(callback);
}

}  // namespace foros
}  // namespace failover
}  // namespace akit
