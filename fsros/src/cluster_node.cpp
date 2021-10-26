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
#include <rclcpp/node_interfaces/node_logging.hpp>
#include <rclcpp/node_interfaces/node_services.hpp>
#include <rclcpp/node_interfaces/node_topics.hpp>

#include <memory>
#include <string>
#include <vector>

#include "cluster_node_impl.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

ClusterNode::ClusterNode(const std::string &node_name,
                         const std::string &cluster_name,
                         const std::vector<std::string> &cluster_node_names,
                         const rclcpp::NodeOptions &options)
    : node_base_(new rclcpp::node_interfaces::NodeBase(
          node_name, cluster_name, options.context(),
          *(options.get_rcl_node_options()), options.use_intra_process_comms(),
          options.enable_topic_statistics())),
      node_logging_(new rclcpp::node_interfaces::NodeLogging(node_base_.get())),
      node_timers_(new rclcpp::node_interfaces::NodeTimers(node_base_.get())),
      node_topics_(new rclcpp::node_interfaces::NodeTopics(node_base_.get(),
                                                           node_timers_.get())),
      node_services_(
          new rclcpp::node_interfaces::NodeServices(node_base_.get())),
      impl_(std::make_unique<ClusterNodeImpl>(cluster_name, node_name,
                                              cluster_node_names, node_base_,
                                              node_services_, *this)) {}

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

rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
ClusterNode::get_node_topics_interface() {
  return node_topics_;
}

rclcpp::node_interfaces::NodeServicesInterface::SharedPtr
ClusterNode::get_node_services_interface() {
  return node_services_;
}

void ClusterNode::add_publisher(
    std::shared_ptr<ClusterNodeInterface> publisher) {
  impl_->add_publisher(publisher);
}

void ClusterNode::remove_publisher(
    std::shared_ptr<ClusterNodeInterface> publisher) {
  impl_->remove_publisher(publisher);
}

void ClusterNode::on_activated() {}

void ClusterNode::on_deactivated() {}

void ClusterNode::on_standby() {}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
