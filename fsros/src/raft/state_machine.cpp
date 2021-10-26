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

#include "raft/state_machine.hpp"

#include <memory>
#include <string>
#include <vector>

#include "fsros_msgs/srv/append_entries.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

StateMachine::StateMachine(
    const std::string &node_name, const std::string &cluster_name,
    const std::vector<std::string> &cluster_node_names,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services)
    : common::StateMachine<State, StateType, Event>(
          StateType::kStandby,
          {{StateType::kStandby, std::make_shared<Standby>()},
           {StateType::kFollower, std::make_shared<Follower>()},
           {StateType::kCandidate, std::make_shared<Candidate>()},
           {StateType::kLeader, std::make_shared<Leader>()}}),
      node_base_(node_base),
      node_services_(node_services),
      service_prefix_("/" + cluster_name + "/" + node_name) {
  initialize_services();
  initialize_clients(cluster_node_names);
}

void StateMachine::initialize_services() {
  append_entries_callback_.set(std::bind(
      &StateMachine::on_append_entries_requested, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  append_entries_service_ =
      std::make_shared<rclcpp::Service<fsros_msgs::srv::AppendEntries>>(
          node_base_->get_shared_rcl_node_handle(),
          service_prefix_ + "/append_entries", append_entries_callback_,
          service_options_);
  node_services_->add_service(
      std::dynamic_pointer_cast<rclcpp::ServiceBase>(append_entries_service_),
      nullptr);

  request_vote_callback_.set(std::bind(
      &StateMachine::on_request_vote_requested, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  request_vote_service_ =
      std::make_shared<rclcpp::Service<fsros_msgs::srv::RequestVote>>(
          node_base_->get_shared_rcl_node_handle(),
          service_prefix_ + "/request_vote", request_vote_callback_,
          service_options_);
  node_services_->add_service(
      std::dynamic_pointer_cast<rclcpp::ServiceBase>(request_vote_service_),
      nullptr);
}

void StateMachine::initialize_clients(const std::vector<std::string> &) {
  // TODO(wonguk.jeong): initialze clients of nodes in cluster
}

void StateMachine::on_append_entries_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<fsros_msgs::srv::AppendEntries::Request>,
    std::shared_ptr<fsros_msgs::srv::AppendEntries::Response>) {}

void StateMachine::on_request_vote_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<fsros_msgs::srv::RequestVote::Request>,
    std::shared_ptr<fsros_msgs::srv::RequestVote::Response>) {}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
