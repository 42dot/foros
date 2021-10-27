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

#include "common/context.hpp"

#include <iostream>
#include <random>
#include <utility>

namespace akit {
namespace failsafe {
namespace fsros {

Context::Context(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    unsigned int election_timeout_min, unsigned int election_timeout_max)
    : node_base_(node_base),
      node_graph_(node_graph),
      node_services_(node_services),
      node_timers_(node_timers),
      node_clock_(node_clock),
      election_timeout_min_(election_timeout_min),
      election_timeout_max_(election_timeout_max),
      random_generator_(random_device_()) {}

rclcpp::TimerBase::SharedPtr Context::create_election_timer(
    rclcpp::VoidCallbackType&& callback) {
  std::uniform_int_distribution<> dist(election_timeout_min_,
                                       election_timeout_max_);

  auto period = dist(random_generator_);

  std::cout << "[" << node_base_->get_name()
            << "] Election Timeout Timer Period: " << period << std::endl;

  auto timer = rclcpp::GenericTimer<rclcpp::VoidCallbackType>::make_shared(
      node_clock_->get_clock(), std::chrono::milliseconds(period),
      std::forward<rclcpp::VoidCallbackType>(callback),
      node_base_->get_context());

  node_timers_->add_timer(timer, nullptr);
  return timer;
}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
