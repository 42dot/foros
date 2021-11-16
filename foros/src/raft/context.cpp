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

#include <foros_msgs/srv/request_vote.hpp>

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "common/node_util.hpp"
#include "common/void_callback.hpp"
#include "raft/state_machine_interface.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

Context::Context(
    const std::string &cluster_name, const uint32_t node_id,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    unsigned int election_timeout_min, unsigned int election_timeout_max,
    ClusterNodeDataInterface &data_interface)
    : cluster_name_(cluster_name),
      node_id_(node_id),
      node_base_(node_base),
      node_graph_(node_graph),
      node_services_(node_services),
      node_timers_(node_timers),
      node_clock_(node_clock),
      current_term_(0),
      voted_(false),
      vote_received_(0),
      available_candidates_(0),
      last_commit_(CommitInfo(0, 0)),
      last_applied_(CommitInfo(0, 0)),
      election_timeout_min_(election_timeout_min),
      election_timeout_max_(election_timeout_max),
      random_generator_(random_device_()),
      broadcast_timeout_(election_timeout_min_ / 10),
      broadcast_received_(false),
      data_interface_(data_interface) {}

void Context::initialize(const std::vector<uint32_t> &cluster_node_ids,
                         StateMachineInterface *state_machine_interface) {
  initialize_node();
  auto data = data_interface_.on_data_get_requested();
  if (data != nullptr) {
    last_commit_.index_ = data->index_;
    last_commit_.term_ = data->term_;
    last_applied_.index_ = data->index_;
    last_applied_.term_ = data->term_;
  }

  initialize_other_nodes(cluster_node_ids);
  state_machine_interface_ = state_machine_interface;
}

void Context::initialize_node() {
  rcl_service_options_t options = rcl_service_get_default_options();

  append_entries_callback_.set(std::bind(
      &Context::on_append_entries_requested, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  append_entries_service_ =
      std::make_shared<rclcpp::Service<foros_msgs::srv::AppendEntries>>(
          node_base_->get_shared_rcl_node_handle(),
          NodeUtil::get_service_name(cluster_name_, node_id_,
                                     NodeUtil::kAppendEntriesServiceName),
          append_entries_callback_, options);

  node_services_->add_service(
      std::dynamic_pointer_cast<rclcpp::ServiceBase>(append_entries_service_),
      nullptr);

  request_vote_callback_.set(std::bind(
      &Context::on_request_vote_requested, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));

  request_vote_service_ =
      std::make_shared<rclcpp::Service<foros_msgs::srv::RequestVote>>(
          node_base_->get_shared_rcl_node_handle(),
          NodeUtil::get_service_name(cluster_name_, node_id_,
                                     NodeUtil::kRequestVoteServiceName),
          request_vote_callback_, options);

  node_services_->add_service(
      std::dynamic_pointer_cast<rclcpp::ServiceBase>(request_vote_service_),
      nullptr);
}

void Context::initialize_other_nodes(
    const std::vector<uint32_t> &cluster_node_ids) {
  rcl_client_options_t options = rcl_client_get_default_options();
  options.qos = rmw_qos_profile_services_default;

  int64_t next_index = last_commit_.index_ + 1;

  for (auto id : cluster_node_ids) {
    if (id == node_id_) {
      continue;
    }

    other_nodes_.push_back(
        std::make_shared<OtherNode>(node_base_, node_graph_, node_services_,
                                    cluster_name_, id, next_index));
  }
}

bool Context::update_term(uint64_t term) {
  if (term <= current_term_) return false;

  current_term_ = term;
  available_candidates_ = 0;
  reset_vote();
  state_machine_interface_->on_new_term_received();

  return true;
}

void Context::on_append_entries_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<foros_msgs::srv::AppendEntries::Request> request,
    std::shared_ptr<foros_msgs::srv::AppendEntries::Response> response) {
  if (request->term < current_term_) {
    response->success = false;
  } else {
    update_term(request->term);
    broadcast_received_ = true;
    state_machine_interface_->on_leader_discovered();
    auto data = data_interface_.on_data_get_requested(request->prev_data_index);
    if (data == nullptr) {
      response->success = false;
    } else {
      if (data->term_ != request->prev_data_term) {
        data_interface_.on_data_rollback_requested(data->index_);
        response->success = false;
      } else {
        response->success = true;
      }
    }
  }

  response->term = current_term_;
}

void Context::on_request_vote_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<foros_msgs::srv::RequestVote::Request> request,
    std::shared_ptr<foros_msgs::srv::RequestVote::Response> response) {
  update_term(request->term);
  std::tie(response->term, response->vote_granted) =
      vote(request->term, request->candidate_id, request->last_data_index,
           request->loat_data_term);
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

std::tuple<uint64_t, bool> Context::vote(uint64_t term, uint32_t id,
                                         uint64_t last_data_index, uint64_t) {
  bool granted = false;

  if (term >= current_term_) {
    if (voted_ == false && last_commit_.index_ <= last_data_index) {
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

std::string Context::get_node_name() { return node_base_->get_name(); }

uint64_t Context::get_term() { return current_term_; }

void Context::broadcast() {
  available_candidates_ = 1;

  for (auto node : other_nodes_) {
    if (node->broadcast(
            current_term_, node_id_,
            std::bind(&Context::on_commit_response, this, std::placeholders::_1,
                      std::placeholders::_2)) == true) {
      available_candidates_++;
    }
  }
}

void Context::request_vote() {
  available_candidates_ = 1;

  for (auto node : other_nodes_) {
    if (node->request_vote(
            current_term_, node_id_,
            std::bind(&Context::on_request_vote_response, this,
                      std::placeholders::_1, std::placeholders::_2)) == true) {
      available_candidates_++;
    }
  }

  if (available_candidates_ <= 1) {
    check_elected();
  }
}

void Context::on_request_vote_response(uint64_t term, bool vote_granted) {
  if (term < current_term_) {
    std::cout << "ignore vote response since term is outdated" << std::endl;
    return;
  }

  if (update_term(term) == true) {
    std::cout << "ignore vote response since term is new one" << std::endl;
    return;
  }

  if (vote_granted == false) {
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

DataCommitResponseSharedFuture Context::commit_data(
    Data::SharedPtr data, DataCommitResponseCallback callback) {
  DataCommitResponseSharedPromise commit_promise =
      std::make_shared<DataCommitResponsePromise>();
  DataCommitResponseSharedFuture commit_future = commit_promise->get_future();

  if (data->index_ <= last_commit_.index_) {
    std::cerr << "out-dated commit index can not be commited" << std::endl;
    auto response = DataCommitResponse::make_shared();
    response->commit_index_ = data->index_;
    response->result_ = false;
    commit_promise->set_value(response);
    callback(commit_future);
    return commit_future;
  }

  auto request_count = request_commit(data);

  if (request_count <= 0) {
    std::cerr << "No other node exist to commit" << std::endl;
    auto response = DataCommitResponse::make_shared();
    response->commit_index_ = data->index_;
    response->result_ = true;
    commit_promise->set_value(response);
    callback(commit_future);
    return commit_future;
  }

  std::lock_guard<std::mutex> lock(pending_commits_mutex_);
  pending_commits_[data->index_] = std::make_tuple(
      commit_promise, std::forward<DataCommitResponseCallback>(callback),
      commit_future,
      std::make_shared<CommitInfo>(data->index_, current_term_, request_count));

  return commit_future;
}

uint64_t Context::get_data_commit_index() { return last_commit_.index_; }

unsigned int Context::request_commit(Data::SharedPtr data) {
  unsigned int request_count = 0;

  for (auto node : other_nodes_) {
    if (node->commit(current_term_, node_id_, data,
                     std::bind(&Context::on_commit_response, this,
                               std::placeholders::_1, std::placeholders::_2)) ==
        true) {
      request_count++;
    }
  }

  return request_count;
}

void Context::on_commit_response(uint64_t term, bool) {
  if (term < current_term_) {
    std::cout << "ignore append entries response since term is outdated"
              << std::endl;
    return;
  }
  update_term(term);
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
