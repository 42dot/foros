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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_

#include <fsros_msgs/srv/append_entries.hpp>
#include <fsros_msgs/srv/request_vote.hpp>
#include <rclcpp/any_service_callback.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common/observer.hpp"
#include "common/state_machine.hpp"
#include "raft/event.hpp"
#include "raft/state.hpp"
#include "raft/state/candidate.hpp"
#include "raft/state/follower.hpp"
#include "raft/state/leader.hpp"
#include "raft/state/standby.hpp"
#include "raft/state_type.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

namespace common = akit::failsafe::fsros::common;

class StateMachine : public common::StateMachine<State, StateType, Event> {
 public:
  explicit StateMachine(
      const std::string &node_name, const std::string &cluster_name,
      const std::vector<std::string> &cluster_node_names,
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services);

  void initialize_services();
  void initialize_clients(const std::vector<std::string> &cluster_node_names);

  void on_append_entries_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<fsros_msgs::srv::AppendEntries::Request> request,
      std::shared_ptr<fsros_msgs::srv::AppendEntries::Response> response);

  void on_request_vote_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<fsros_msgs::srv::RequestVote::Request> request,
      std::shared_ptr<fsros_msgs::srv::RequestVote::Response> response);

 private:
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services_;
  std::shared_ptr<rclcpp::Service<fsros_msgs::srv::AppendEntries>>
      append_entries_service_;
  rclcpp::AnyServiceCallback<fsros_msgs::srv::AppendEntries>
      append_entries_callback_;
  std::shared_ptr<rclcpp::Service<fsros_msgs::srv::RequestVote>>
      request_vote_service_;
  rclcpp::AnyServiceCallback<fsros_msgs::srv::RequestVote>
      request_vote_callback_;
  rcl_service_options_t service_options_ = rcl_service_get_default_options();
  std::string service_prefix_;
};

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
