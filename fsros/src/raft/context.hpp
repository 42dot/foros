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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_CONTEXT_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_CONTEXT_HPP_

#include <fsros_msgs/srv/append_entries.hpp>
#include <fsros_msgs/srv/request_vote.hpp>
#include <rclcpp/any_service_callback.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/timer.hpp>

#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "raft/state_machine_interface.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

class Context {
 public:
  Context(
      const std::string &cluster_name, const uint32_t node_id,
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
      rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
      rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
      unsigned int election_timeout_min, unsigned int election_timeout_max);

  void initialize(const std::vector<uint32_t> &cluster_node_ids,
                  StateMachineInterface *state_machine_interface);
  void start_election_timer();
  void stop_election_timer();
  void reset_election_timer();
  void start_broadcast_timer();
  void stop_broadcast_timer();
  void reset_broadcast_timer();
  std::string get_node_name();
  void request_vote();
  void vote_for_me();
  std::tuple<uint64_t, bool> vote(uint64_t term, uint32_t id);
  void reset_vote();
  void increase_term();
  uint64_t get_term();
  void broadcast();

 private:
  void initialize_services();
  void initialize_clients(const std::vector<uint32_t> &cluster_node_ids);
  void on_append_entries_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<fsros_msgs::srv::AppendEntries::Request> request,
      std::shared_ptr<fsros_msgs::srv::AppendEntries::Response> response);
  void on_append_entries_response(
      rclcpp::Client<fsros_msgs::srv::AppendEntries>::SharedFutureWithRequest
          future);
  void on_request_vote_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<fsros_msgs::srv::RequestVote::Request> request,
      std::shared_ptr<fsros_msgs::srv::RequestVote::Response> response);
  void on_request_vote_response(
      rclcpp::Client<fsros_msgs::srv::RequestVote>::SharedFutureWithRequest
          future);
  bool update_term(uint64_t term);
  void check_elected();

  const std::string cluster_name_;
  uint32_t node_id_;

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph_;
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services_;
  rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers_;
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock_;

  std::shared_ptr<rclcpp::Service<fsros_msgs::srv::AppendEntries>>
      append_entries_service_;
  rclcpp::AnyServiceCallback<fsros_msgs::srv::AppendEntries>
      append_entries_callback_;
  std::vector<std::shared_ptr<rclcpp::Client<fsros_msgs::srv::AppendEntries>>>
      append_entries_clients_ = {};
  std::shared_ptr<rclcpp::Service<fsros_msgs::srv::RequestVote>>
      request_vote_service_;
  rclcpp::AnyServiceCallback<fsros_msgs::srv::RequestVote>
      request_vote_callback_;
  std::vector<std::shared_ptr<rclcpp::Client<fsros_msgs::srv::RequestVote>>>
      request_vote_clients_ = {};

  uint64_t current_term_;
  uint32_t voted_for_;
  bool voted_;
  unsigned int vote_received_;
  unsigned int available_candidates_;

  unsigned int election_timeout_min_;
  unsigned int election_timeout_max_;
  std::random_device random_device_;
  std::mt19937 random_generator_;
  rclcpp::TimerBase::SharedPtr election_timer_;

  unsigned int broadcast_timeout_;
  rclcpp::TimerBase::SharedPtr broadcast_timer_;

  StateMachineInterface *state_machine_interface_;

  bool broadcast_received_;
};

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_CONTEXT_HPP_
