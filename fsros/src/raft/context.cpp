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

#include <fsros_msgs/srv/request_vote.hpp>

#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <tuple>
#include <utility>

#include "common/void_callback.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

Context::Context(
    const uint32_t node_id,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    unsigned int election_timeout_min, unsigned int election_timeout_max)
    : node_id_(node_id),
      node_base_(node_base),
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

void Context::reset_election_timer() {
  stop_election_timer();
  start_election_timer();
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

void Context::vote_for_me() {
  voted_for_ = node_id_;
  voted_ = true;
}

std::tuple<uint64_t, bool> Context::vote(uint64_t term, uint32_t id) {
  bool granted = false;

  if (term >= current_term_) {
    if (voted_ == false) {
      voted_for_ = id;
      voted_ = true;
      granted = true;
    } else if (voted_for_ == id) {
      granted = true;
    }
  }

  return std::make_tuple(current_term_, granted);
}

void Context::reset_vote() {
  voted_for_ = 0;
  voted_ = false;
}

void Context::increase_term() {
  current_term_++;
  std::cout << "[" << node_base_->get_name()
            << "] term increased: " << current_term_ << std::endl;
}

void Context::request_vote() {
  for (auto client : request_vote_clients_) {
    auto request = std::make_shared<fsros_msgs::srv::RequestVote::Request>();
    request->term = current_term_;
    request->candidate_id = node_id_;
    auto response = client->async_send_request(
        request, std::bind(&Context::on_request_vote_response, this,
                           std::placeholders::_1));
  }
}

void Context::on_request_vote_response(
    rclcpp::Client<fsros_msgs::srv::RequestVote>::SharedFutureWithRequest
        future) {
  std::cout << "vote response" << std::endl;
  auto ret = future.get();
  auto response = ret.second;

  std::cout << "response: " << response->term << std::endl;
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
