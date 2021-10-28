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

#include "raft/context.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <utility>

#include "common/void_callback.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

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

void Context::start_election_timer() {
  if (election_timer_ != nullptr) {
    std::cerr << "[" << node_base_->get_name() << "] Election timer exist"
              << std::endl;
  }

  std::uniform_int_distribution<> dist(election_timeout_min_,
                                       election_timeout_max_);
  auto period = dist(random_generator_);
  std::cout << "[" << node_base_->get_name()
            << "] Election timeout period: " << period << std::endl;

  election_timer_ = rclcpp::GenericTimer<rclcpp::VoidCallbackType>::make_shared(
      node_clock_->get_clock(), std::chrono::milliseconds(period),
      std::forward<rclcpp::VoidCallbackType>(
          std::bind(&Context::on_election_timer_expired, this)),
      node_base_->get_context());
  node_timers_->add_timer(election_timer_, nullptr);
}

void Context::stop_election_timer() {
  if (election_timer_ != nullptr) {
    election_timer_->cancel();
    election_timer_.reset();
  }
}

std::weak_ptr<VoidCallback> Context::add_election_timer_callback(
    std::function<void()> callback) {
  auto handle = std::make_shared<VoidCallback>(callback);
  election_timer_callbacks_.emplace_back(handle);
  return handle;
}

void Context::remove_election_timer_callback(
    std::weak_ptr<VoidCallback> handle) {
  auto callback = handle.lock();
  election_timer_callbacks_.remove(callback);
}

void Context::on_election_timer_expired() {
  for (auto cb : election_timer_callbacks_) {
    if (cb != nullptr) {
      cb->call();
    }
  }
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
