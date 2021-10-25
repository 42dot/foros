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

namespace akit {
namespace failsafe {
namespace fsros {

ClusterNodeImpl::ClusterNodeImpl(LifecycleListener &lifecycle_listener,
                                 const std::string &node_name,
                                 const std::string &node_namespace,
                                 const rclcpp::NodeOptions &options)
    : node_base_(new rclcpp::node_interfaces::NodeBase(
          node_name, node_namespace, options.context(),
          *(options.get_rcl_node_options()), options.use_intra_process_comms(),
          options.enable_topic_statistics())),
      raft_fsm_(std::make_unique<raft::StateMachine>()),
      lifecycle_fsm_(std::make_unique<lifecycle::StateMachine>()),
      lifecycle_listener_(lifecycle_listener) {
  lifecycle_fsm_->Subscribe(this);
}

void ClusterNodeImpl::handle(const lifecycle::StateType &state) {
  switch (state) {
    case lifecycle::StateType::kStandby:
      lifecycle_listener_.on_standby();
      break;
    case lifecycle::StateType::kActive:
      lifecycle_listener_.on_activated();
      break;
    case lifecycle::StateType::kInactive:
      lifecycle_listener_.on_deactivated();
      break;
    default:
      std::cerr << "Invalid lifecycle state : " << static_cast<int>(state)
                << std::endl;
      break;
  }
}

void ClusterNodeImpl::handle(const raft::StateType &state) {
  switch (state) {
    case raft::StateType::kStandby:
      break;
    case raft::StateType::kFollower:
      break;
    case raft::StateType::kCandidate:
      break;
    case raft::StateType::kLeader:
      break;
    default:
      std::cerr << "Invalid raft state : " << static_cast<int>(state)
                << std::endl;
      break;
  }
}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
