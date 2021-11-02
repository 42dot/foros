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

#include "akit/failsafe/fsros/cluster_node.hpp"

#include <rclcpp/node_interfaces/node_base.hpp>
#include <rclcpp/node_interfaces/node_clock.hpp>
#include <rclcpp/node_interfaces/node_graph.hpp>
#include <rclcpp/node_interfaces/node_logging.hpp>
#include <rclcpp/node_interfaces/node_services.hpp>
#include <rclcpp/node_interfaces/node_topics.hpp>

#include <memory>
#include <string>
#include <vector>

#include "akit/failsafe/fsros/cluster_node_options.hpp"
#include "cluster_node_impl.hpp"
#include "common/node_util.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

ClusterNode::ClusterNode(const uint32_t node_id,
                         const std::string &cluster_name,
                         const std::vector<uint32_t> &cluster_node_ids,
                         const ClusterNodeOptions &options)
    : node_base_(new rclcpp::node_interfaces::NodeBase(
          NodeUtil::get_node_name(node_id), cluster_name,
          options.node_options.context(),
          *(options.node_options.get_rcl_node_options()),
          options.node_options.use_intra_process_comms(),
          options.node_options.enable_topic_statistics())),
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
      impl_(std::make_unique<ClusterNodeImpl>(
          node_id, cluster_node_ids, node_base_, node_graph_, node_services_,
          node_timers_, node_clock_, *this, options)) {}

ClusterNode::~ClusterNode() {}

const char *ClusterNode::get_name() const { return node_base_->get_name(); }

const char *ClusterNode::get_namespace() const {
  return node_base_->get_namespace();
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

rclcpp::node_interfaces::NodeBaseInterface::SharedPtr
ClusterNode::get_node_base_interface() {
  return node_base_;
}

rclcpp::node_interfaces::NodeGraphInterface::SharedPtr
ClusterNode::get_node_graph_interface() {
  return node_graph_;
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

rclcpp::node_interfaces::NodeClockInterface::SharedPtr
ClusterNode::get_node_clock_interface() {
  return node_clock_;
}

void ClusterNode::on_activated() {}

void ClusterNode::on_deactivated() {}

void ClusterNode::on_standby() {}

bool ClusterNode::is_activated() { return impl_->is_activated(); }

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
