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

#include "cluster_node_impl.hpp"

#include <rclcpp/node_interfaces/node_base.hpp>

#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node_options.hpp"
#include "raft/context.hpp"

namespace akit {
namespace failover {
namespace foros {

ClusterNodeImpl::ClusterNodeImpl(
    const std::string &cluster_name, const uint32_t node_id,
    const std::vector<uint32_t> &cluster_node_ids,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    const ClusterNodeOptions &options)
    : logger_(node_logging->get_logger().get_child("cluster_node")),
      raft_context_(std::make_shared<raft::Context>(
          cluster_name, node_id, node_base, node_graph, node_services,
          node_topics, node_timers, node_clock, options.election_timeout_min(),
          options.election_timeout_max(), options.temp_directory(), logger_)),
      raft_fsm_(std::make_unique<raft::StateMachine>(cluster_node_ids,
                                                     raft_context_, logger_)),
      lifecycle_fsm_(std::make_unique<lifecycle::StateMachine>(logger_)) {
  lifecycle_fsm_->subscribe(this);
  raft_fsm_->subscribe(this);
  raft_fsm_->handle(raft::Event::kStarted);
}

ClusterNodeImpl::~ClusterNodeImpl() {
  lifecycle_fsm_->unsubscribe(this);
  raft_fsm_->unsubscribe(this);
}

void ClusterNodeImpl::handle(const lifecycle::StateType &state) {
  switch (state) {
    case lifecycle::StateType::kStandby:
      if (standby_callback_) {
        standby_callback_();
      }
      break;
    case lifecycle::StateType::kActive:
      if (activated_callback_) {
        activated_callback_();
      }
      break;
    case lifecycle::StateType::kInactive:
      if (deactivated_callback_) {
        deactivated_callback_();
      }
      break;
    default:
      RCLCPP_ERROR(logger_, "Invalid lifecycle state : %d",
                   static_cast<int>(state));

      break;
  }
}

void ClusterNodeImpl::handle(const raft::StateType &state) {
  switch (state) {
    case raft::StateType::kStandby:
      lifecycle_fsm_->handle(lifecycle::Event::kDeactivate);
      break;
    case raft::StateType::kFollower:
      lifecycle_fsm_->handle(lifecycle::Event::kStandby);
      break;
    case raft::StateType::kCandidate:
      lifecycle_fsm_->handle(lifecycle::Event::kStandby);
      break;
    case raft::StateType::kLeader:
      lifecycle_fsm_->handle(lifecycle::Event::kActivate);
      break;
    default:
      RCLCPP_ERROR(logger_, "Invalid raft state (%lu) : %d",
                   raft_context_->get_term(), static_cast<int>(state));
      break;
  }
}

bool ClusterNodeImpl::is_activated() {
  return lifecycle_fsm_->get_current_state_type() ==
         lifecycle::StateType::kActive;
}

CommandCommitResponseSharedFuture ClusterNodeImpl::commit_command(
    Command::SharedPtr command, CommandCommitResponseCallback &callback) {
  return raft_context_->commit_command(command, callback);
}

void ClusterNodeImpl::register_on_activated(std::function<void()> callback) {
  set_activated_callback(callback);
}

void ClusterNodeImpl::register_on_deactivated(std::function<void()> callback) {
  set_deactivated_callback(callback);
}

void ClusterNodeImpl::register_on_standby(std::function<void()> callback) {
  set_standby_callback(callback);
}

void ClusterNodeImpl::set_activated_callback(std::function<void()> callback) {
  activated_callback_ = callback;
}

void ClusterNodeImpl::set_deactivated_callback(std::function<void()> callback) {
  deactivated_callback_ = callback;
}

void ClusterNodeImpl::set_standby_callback(std::function<void()> callback) {
  standby_callback_ = callback;
}

uint64_t ClusterNodeImpl::get_commands_size() {
  return raft_context_->get_commands_size();
}

Command::SharedPtr ClusterNodeImpl::get_command(uint64_t id) {
  return raft_context_->get_command(id);
}

void ClusterNodeImpl::register_on_committed(
    std::function<void(const uint64_t, Command::SharedPtr)> callback) {
  raft_context_->register_on_committed(callback);
}

void ClusterNodeImpl::register_on_reverted(
    std::function<void(const uint64_t)> callback) {
  raft_context_->register_on_reverted(callback);
}

}  // namespace foros
}  // namespace failover
}  // namespace akit
