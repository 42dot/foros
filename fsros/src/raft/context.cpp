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

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "common/node_util.hpp"
#include "common/void_callback.hpp"
#include "raft/state_machine_interface.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

Context::Context(
    const std::string &cluster_name, const uint32_t node_id,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    unsigned int election_timeout_min, unsigned int election_timeout_max)
    : cluster_name_(cluster_name),
      node_id_(node_id),
      node_base_(node_base),
      node_graph_(node_graph),
      node_services_(node_services),
      node_timers_(node_timers),
      node_clock_(node_clock),
      current_term_(0),
      voted_(false),
      election_timeout_min_(election_timeout_min),
      election_timeout_max_(election_timeout_max),
      random_generator_(random_device_()),
      broadcast_timeout_(election_timeout_min_ / 10),
      broadcast_received_(false) {}

void Context::initialize(const std::vector<uint32_t> &cluster_node_ids,
                         StateMachineInterface *state_machine_interface) {
  initialize_services();
  initialize_clients(cluster_node_ids);
  state_machine_interface_ = state_machine_interface;
}

void Context::initialize_services() {
  rcl_service_options_t options = rcl_service_get_default_options();

  append_entries_callback_.set(std::bind(
      &Context::on_append_entries_requested, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  append_entries_service_ =
      std::make_shared<rclcpp::Service<fsros_msgs::srv::AppendEntries>>(
          node_base_->get_shared_rcl_node_handle(),
          NodeUtil::get_service_name(cluster_name_, node_id_,
                                     kAppendEntriesServiceName),
          append_entries_callback_, options);

  node_services_->add_service(
      std::dynamic_pointer_cast<rclcpp::ServiceBase>(append_entries_service_),
      nullptr);

  request_vote_callback_.set(std::bind(
      &Context::on_request_vote_requested, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  request_vote_service_ =
      std::make_shared<rclcpp::Service<fsros_msgs::srv::RequestVote>>(
          node_base_->get_shared_rcl_node_handle(),
          NodeUtil::get_service_name(cluster_name_, node_id_,
                                     kRequestVoteServiceName),
          request_vote_callback_, options);

  node_services_->add_service(
      std::dynamic_pointer_cast<rclcpp::ServiceBase>(request_vote_service_),
      nullptr);
}

void Context::initialize_clients(
    const std::vector<uint32_t> &cluster_node_ids) {
  rcl_client_options_t options = rcl_client_get_default_options();
  options.qos = rmw_qos_profile_services_default;

  for (auto id : cluster_node_ids) {
    if (id == node_id_) {
      continue;
    }

    auto append_entries =
        rclcpp::Client<fsros_msgs::srv::AppendEntries>::make_shared(
            node_base_.get(), node_graph_,
            NodeUtil::get_service_name(cluster_name_, id,
                                       kAppendEntriesServiceName),
            options);
    node_services_->add_client(
        std::dynamic_pointer_cast<rclcpp::ClientBase>(append_entries), nullptr);
    append_entries_clients_.push_back(append_entries);

    auto request_vote =
        rclcpp::Client<fsros_msgs::srv::RequestVote>::make_shared(
            node_base_.get(), node_graph_,
            NodeUtil::get_service_name(cluster_name_, id,
                                       kRequestVoteServiceName),
            options);
    node_services_->add_client(
        std::dynamic_pointer_cast<rclcpp::ClientBase>(request_vote), nullptr);
    request_vote_clients_.push_back(request_vote);
  }
}

bool Context::update_term(uint64_t term) {
  if (term <= current_term_) return false;

  current_term_ = term;
  reset_vote();
  state_machine_interface_->on_new_term_received();

  return true;
}

void Context::on_append_entries_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<fsros_msgs::srv::AppendEntries::Request> request,
    std::shared_ptr<fsros_msgs::srv::AppendEntries::Response> response) {
  if (request->term < current_term_) {
    response->success = false;
  } else {
    update_term(request->term);
    response->success = true;
    broadcast_received_ = true;
    state_machine_interface_->on_leader_discovered();
  }

  response->term = current_term_;
}

void Context::on_request_vote_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<fsros_msgs::srv::RequestVote::Request> request,
    std::shared_ptr<fsros_msgs::srv::RequestVote::Response> response) {
  update_term(request->term);
  std::tie(response->term, response->vote_granted) =
      vote(request->term, request->candidate_id);
}

void Context::start_election_timer() {
  if (election_timer_ != nullptr) {
    election_timer_->cancel();
    election_timer_.reset();
  }

  std::uniform_int_distribution<> dist(election_timeout_min_,
                                       election_timeout_max_);
  auto period = dist(random_generator_);

  election_timer_ = rclcpp::GenericTimer<rclcpp::VoidCallbackType>::make_shared(
      node_clock_->get_clock(), std::chrono::milliseconds(period),
      [this]() {
        if (broadcast_received_ == true) {
          broadcast_received_ = false;
          return;
        }
        state_machine_interface_->on_election_timedout();
      },
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

void Context::start_broadcast_timer() {
  if (broadcast_timer_ != nullptr) {
    broadcast_timer_->cancel();
    broadcast_timer_.reset();
  }

  broadcast_timer_ =
      rclcpp::GenericTimer<rclcpp::VoidCallbackType>::make_shared(
          node_clock_->get_clock(),
          std::chrono::milliseconds(broadcast_timeout_),
          [this]() { state_machine_interface_->on_broadcast_timedout(); },
          node_base_->get_context());
  node_timers_->add_timer(broadcast_timer_, nullptr);
}

void Context::stop_broadcast_timer() {
  if (broadcast_timer_ != nullptr) {
    broadcast_timer_->cancel();
    broadcast_timer_.reset();
  }
}

void Context::reset_broadcast_timer() {
  stop_broadcast_timer();
  start_broadcast_timer();
}

void Context::vote_for_me() {
  voted_for_ = node_id_;
  vote_received_ = 1;
  voted_ = true;
}

std::tuple<uint64_t, bool> Context::vote(uint64_t term, uint32_t id) {
  bool granted = false;

  if (term >= current_term_) {
    if (voted_ == false) {
      voted_for_ = id;
      voted_ = true;
      granted = true;
    }
  }

  return std::make_tuple(current_term_, granted);
}

void Context::reset_vote() {
  voted_for_ = 0;
  vote_received_ = 0;
  voted_ = false;
}

void Context::increase_term() { current_term_++; }

void Context::request_vote() {
  available_candidates_ = 1;

  for (auto client : request_vote_clients_) {
    if (client->service_is_ready() == false) {
      continue;
    }

    available_candidates_++;

    auto request = std::make_shared<fsros_msgs::srv::RequestVote::Request>();
    request->term = current_term_;
    request->candidate_id = node_id_;
    auto response = client->async_send_request(
        request, std::bind(&Context::on_request_vote_response, this,
                           std::placeholders::_1));
  }

  if (available_candidates_ <= 1) {
    check_elected();
  }
}

void Context::on_request_vote_response(
    rclcpp::Client<fsros_msgs::srv::RequestVote>::SharedFutureWithRequest
        future) {
  auto ret = future.get();
  auto response = ret.second;

  if (response->term < current_term_) {
    std::cout << "ignore vote response since term is outdated" << std::endl;
    return;
  }

  if (update_term(response->term) == true) {
    std::cout << "ignore vote response since term is new one" << std::endl;
    return;
  }

  if (response->vote_granted == false) {
    std::cout << "vote not granted" << std::endl;
    return;
  }

  vote_received_++;

  check_elected();
}

void Context::check_elected() {
  auto majority = (available_candidates_ >> 1) + 1;
  if (vote_received_ < majority) return;

  state_machine_interface_->on_elected();
}

std::string Context::get_node_name() { return node_base_->get_name(); }

uint64_t Context::get_term() { return current_term_; }

void Context::broadcast() {
  available_candidates_ = 1;

  for (auto client : append_entries_clients_) {
    if (client->service_is_ready() == false) {
      continue;
    }

    available_candidates_++;

    auto request = std::make_shared<fsros_msgs::srv::AppendEntries::Request>();
    request->term = current_term_;
    request->leader_id = node_id_;
    auto response = client->async_send_request(
        request, std::bind(&Context::on_append_entries_response, this,
                           std::placeholders::_1));
  }
}

void Context::on_append_entries_response(
    rclcpp::Client<fsros_msgs::srv::AppendEntries>::SharedFutureWithRequest
        future) {
  auto ret = future.get();
  auto response = ret.second;

  if (response->term < current_term_) {
    std::cout << "ignore append entries response since term is outdated"
              << std::endl;
    return;
  }

  update_term(response->term);
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
