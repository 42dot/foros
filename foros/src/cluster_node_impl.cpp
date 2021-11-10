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
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    ClusterNodeInterface &node_interface, const ClusterNodeOptions &options)
    : raft_context_(std::make_shared<raft::Context>(
          cluster_name, node_id, node_base, node_graph, node_services,
          node_timers, node_clock, options.election_timeout_min(),
          options.election_timeout_max())),
      raft_fsm_(std::make_unique<raft::StateMachine>(cluster_node_ids,
                                                     raft_context_)),
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

void ClusterNodeImpl::handle(const lifecycle::StateType &state) {
  switch (state) {
    case lifecycle::StateType::kStandby:
      node_interface_.on_standby();
      break;
    case lifecycle::StateType::kActive:
      node_interface_.on_activated();
      break;
    case lifecycle::StateType::kInactive:
      node_interface_.on_deactivated();
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
      std::cout << "raft state: Standby (" << raft_context_->get_term() << ")"
                << std::endl;
      lifecycle_fsm_->handle(lifecycle::Event::kDeactivate);
      break;
    case raft::StateType::kFollower:
      std::cout << "raft state: Follower (" << raft_context_->get_term() << ")"
                << std::endl;
      lifecycle_fsm_->handle(lifecycle::Event::kStandby);
      break;
    case raft::StateType::kCandidate:
      std::cout << "raft state: Candidate (" << raft_context_->get_term() << ")"
                << std::endl;
      lifecycle_fsm_->handle(lifecycle::Event::kStandby);
      break;
    case raft::StateType::kLeader:
      std::cout << "raft state: Leader (" << raft_context_->get_term() << ")"
                << std::endl;
      lifecycle_fsm_->handle(lifecycle::Event::kActivate);
      break;
    default:
      std::cerr << "Invalid raft state (" << raft_context_->get_term()
                << "): " << static_cast<int>(state) << std::endl;
      break;
  }
}

bool ClusterNodeImpl::is_activated() {
  return lifecycle_fsm_->get_current_state_type() ==
         lifecycle::StateType::kActive;
}

CommitResponseSharedFuture ClusterNodeImpl::commit_data(
    CommitData::SharedPtr data, CommitResponseCallback callback) {
  std::unique_lock<std::mutex> lock(pending_commits_mutex_);

  CommitResponseSharedPromise commit_promise =
      std::make_shared<CommitResponsePromise>();
  CommitResponseSharedFuture commit_future = commit_promise->get_future();

  // FIXME: for now, send fake response until commit logic is implemented.
  lock.unlock();
  auto response = CommitResponse::make_shared();
  response->commit_id = data->commit_id;
  response->result = true;
  commit_promise->set_value(response);
  callback(commit_future);

  return commit_future;
}

uint64_t ClusterNodeImpl::get_data_commit_index() {
  return raft_context_->get_commit_index();
}

}  // namespace foros
}  // namespace failover
}  // namespace akit
