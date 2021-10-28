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

#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <random>
#include <vector>

#include "common/void_callback.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

class Context {
 public:
  Context(
      const uint32_t node_id,
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
      rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
      rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
      unsigned int election_timeout_min, unsigned int election_timeout_max);

  void start_election_timer();
  void stop_election_timer();
  void reset_election_timer();
  std::weak_ptr<VoidCallback> add_election_timer_callback(
      std::function<void()> callback);
  void remove_election_timer_callback(std::weak_ptr<VoidCallback> handle);
  void on_election_timer_expired();

  void vote_for_me();
  void increase_term();

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

  uint64_t current_term_ = 0;
  uint32_t voted_for_ = -1;

 private:
  unsigned int election_timeout_min_;
  unsigned int election_timeout_max_;
  std::random_device random_device_;
  std::mt19937 random_generator_;

  rclcpp::TimerBase::SharedPtr election_timer_;
  std::list<std::shared_ptr<VoidCallback>> election_timer_callbacks_ = {};
};

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_CONTEXT_HPP_
