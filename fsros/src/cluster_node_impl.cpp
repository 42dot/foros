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

#include "akit/failsafe/fsros/cluster_node_options.hpp"
#include "common/context.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

ClusterNodeImpl::ClusterNodeImpl(
    const std::vector<std::string> &cluster_node_names,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    ClusterNodeInterface &node_interface, const ClusterNodeOptions &options)
    : context_(std::make_shared<Context>(
          node_base, node_graph, node_services, node_timers, node_clock,
          options.election_timeout.min, options.election_timeout.max)),
      raft_fsm_(
          std::make_unique<raft::StateMachine>(cluster_node_names, context_)),
      lifecycle_fsm_(std::make_unique<lifecycle::StateMachine>()),
      node_interface_(node_interface) {
  lifecycle_fsm_->subscribe(this);
  raft_fsm_->subscribe(this);
  raft_fsm_->handle(raft::Event::kStarted);
}

ClusterNodeImpl::~ClusterNodeImpl() {
  lifecycle_fsm_->unsubscribe(this);
  raft_fsm_->unsubscribe(this);
}

void ClusterNodeImpl::visit_publishers(
    std::function<void(std::shared_ptr<ClusterNodeInterface>)> f) {
  auto pub = publishers_.begin();
  while (pub != publishers_.end()) {
    auto pub_shared = pub->lock();
    if (pub_shared == nullptr) {
      pub = publishers_.erase(pub);
    } else {
      f(pub_shared);
    }
  }
}

void ClusterNodeImpl::handle(const lifecycle::StateType &state) {
  switch (state) {
    case lifecycle::StateType::kStandby:
      node_interface_.on_standby();
      visit_publishers([](auto publisher) { publisher->on_standby(); });
      break;
    case lifecycle::StateType::kActive:
      node_interface_.on_activated();
      visit_publishers([](auto publisher) { publisher->on_activated(); });
      break;
    case lifecycle::StateType::kInactive:
      node_interface_.on_deactivated();
      visit_publishers([](auto publisher) { publisher->on_deactivated(); });
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
      std::cout << "raft state: Standby" << std::endl;
      break;
    case raft::StateType::kFollower:
      std::cout << "raft state: Follower" << std::endl;
      break;
    case raft::StateType::kCandidate:
      std::cout << "raft state: Candidate" << std::endl;
      break;
    case raft::StateType::kLeader:
      std::cout << "raft state: Leader" << std::endl;
      break;
    default:
      std::cerr << "Invalid raft state : " << static_cast<int>(state)
                << std::endl;
      break;
  }
}

void ClusterNodeImpl::add_publisher(
    std::shared_ptr<ClusterNodeInterface> publisher) {
  publishers_.push_back(publisher);
}

void ClusterNodeImpl::remove_publisher(
    std::shared_ptr<ClusterNodeInterface> publisher) {
  auto pub = publishers_.begin();
  while (pub != publishers_.end()) {
    auto pub_shared = pub->lock();
    if (pub_shared == publisher) {
      pub = publishers_.erase(pub);
    }
  }
}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
