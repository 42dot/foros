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

#include <functional>
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
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    const unsigned int election_timeout_min,
    const unsigned int election_timeout_max, const std::string &temp_directory,
    rclcpp::Logger &logger)
    : cluster_name_(cluster_name),
      node_id_(node_id),
      node_base_(node_base),
      node_graph_(node_graph),
      node_services_(node_services),
      node_timers_(node_timers),
      node_clock_(node_clock),
      majority_(0),
      cluster_size_(0),
      election_timeout_min_(election_timeout_min),
      election_timeout_max_(election_timeout_max),
      random_generator_(random_device_()),
      broadcast_timeout_(election_timeout_min_ / 10),
      broadcast_received_(false),
      state_machine_interface_(nullptr),
      logger_(logger.get_child("raft")) {
  auto db_file = temp_directory + "/foros_" + node_base_->get_name();
  store_ = std::make_unique<ContextStore>(db_file, logger_);
  inspector_ = std::make_unique<Inspector>(
      node_base, node_topics, node_timers, node_clock,
      std::bind(&Context::inspector_message_requested, this,
                std::placeholders::_1));
}

void Context::initialize(const std::vector<uint32_t> &cluster_node_ids,
                         StateMachineInterface *state_machine_interface) {
  initialize_node();
  set_cluster_size(cluster_node_ids.size());
  initialize_other_nodes(cluster_node_ids);
  set_state_machine_interface(state_machine_interface);
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

  int64_t next_index = store_->logs_size();

  for (auto id : cluster_node_ids) {
    if (id == node_id_) {
      continue;
    }

    other_nodes_[id] = std::make_shared<OtherNode>(
        node_base_, node_graph_, node_services_, cluster_name_, id, next_index,
        std::bind(&Context::on_log_get_request, this, std::placeholders::_1));
  }
}

void Context::set_cluster_size(uint32_t size) {
  cluster_size_ = size;
  majority_ = (size >> 1) + 1;
}

void Context::set_state_machine_interface(
    StateMachineInterface *state_machine_interface) {
  state_machine_interface_ = state_machine_interface;
}

bool Context::update_term(uint64_t term, bool self) {
  if (term <= store_->current_term()) {
    return false;
  }

  store_->current_term(term);

  reset_vote();
  if (self == false) {
    state_machine_interface_->on_new_term_received();
  }

  return true;
}

void Context::on_append_entries_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<foros_msgs::srv::AppendEntries::Request> request,
    std::shared_ptr<foros_msgs::srv::AppendEntries::Response> response) {
  if (is_valid_node(request->leader_id) == false) {
    response->success = false;
    return;
  }

  response->term = store_->current_term();

  if (request->term < response->term) {
    response->success = false;
  } else {
    update_term(request->term);
    broadcast_received_ = true;
    state_machine_interface_->on_leader_discovered();
  }

  if (request->entries.size() == 0) {
    response->success = false;
    return;
  }

  // commit it since it is first data
  if (request->leader_commit == 0) {
    response->success = request_local_commit(request);
    return;
  }

  auto log = store_->log(request->prev_log_index);
  if (log == nullptr) {
    response->success = false;
  } else {
    if (log->term_ != request->prev_log_term) {
      request_local_rollback(log->id_);
      response->success = false;
    } else {
      response->success = request_local_commit(request);
    }
  }
}

bool Context::request_local_commit(
    const std::shared_ptr<foros_msgs::srv::AppendEntries::Request> request) {
  auto log = store_->log();

  if (log != nullptr) {
    if (log->command_ != nullptr && log->id_ == request->leader_commit &&
        log->term_ == request->term) {
      return true;
    }

    if (log->id_ >= request->leader_commit) {
      store_->revert_log(request->leader_commit);
      invoke_revert_callback(request->leader_commit);
    }
  }

  log = LogEntry::make_shared(request->leader_commit, request->term,
                              Command::make_shared(request->entries));

  if (store_->push_log(log) == false) {
    return false;
  }

  invoke_commit_callback(log);

  return true;
}

void Context::request_local_rollback(const uint64_t commit_index) {
  store_->revert_log(commit_index);
}

void Context::on_request_vote_requested(
    const std::shared_ptr<rmw_request_id_t>,
    const std::shared_ptr<foros_msgs::srv::RequestVote::Request> request,
    std::shared_ptr<foros_msgs::srv::RequestVote::Response> response) {
  if (is_valid_node(request->candidate_id) == false) {
    return;
  }

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
  store_->voted_for(node_id_);
  store_->voted(true);
  store_->increase_vote_received();
}

std::tuple<uint64_t, bool> Context::vote(const uint64_t term, const uint32_t id,
                                         const uint64_t last_data_index,
                                         const uint64_t) {
  bool granted = false;
  auto log = store_->log();
  auto current_term = store_->current_term();

  if (term >= current_term) {
    if (store_->voted() == false &&
        (log == nullptr || log->id_ <= last_data_index)) {
      store_->voted_for(id);
      store_->voted(true);
      granted = true;
    }
  }

  return std::make_tuple(current_term, granted);
}

void Context::reset_vote() {
  store_->voted_for(0);
  store_->reset_vote_received();
  store_->voted(false);
}

void Context::increase_term() { update_term(store_->current_term() + 1, true); }

std::string Context::get_node_name() { return node_base_->get_name(); }

uint64_t Context::get_term() { return store_->current_term(); }

void Context::broadcast() {
  LogEntry::SharedPtr log;

  auto pending_commit = get_pending_commit();
  if (pending_commit != nullptr && pending_commit->log_ != nullptr) {
    log = pending_commit->log_;
  } else {
    log = store_->log();
  }

  for (auto &node : other_nodes_) {
    node.second->broadcast(
        store_->current_term(), node_id_, log,
        std::bind(&Context::on_broadcast_response, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3,
                  std::placeholders::_4));
  }
}

void Context::request_vote() {
  for (auto &node : other_nodes_) {
    node.second->request_vote(
        store_->current_term(), node_id_, store_->log(),
        std::bind(&Context::on_request_vote_response, this,
                  std::placeholders::_1, std::placeholders::_2));
  }
  check_elected();
}

void Context::on_request_vote_response(const uint64_t term,
                                       const bool vote_granted) {
  if (term < store_->current_term()) {
    // ignore vote response since term is outdated
    return;
  }

  if (update_term(term) == true) {
    // ignore vote response since ther is new one
    return;
  }

  if (vote_granted == false) {
    // vote not granted
    return;
  }

  store_->increase_vote_received();
  check_elected();
}

void Context::check_elected() {
  if (store_->vote_received() < majority_) return;

  uint64_t id;
  auto log = store_->log();
  if (log == nullptr) {
    id = 0;
  } else {
    id = log->id_;
  }

  for (auto &node : other_nodes_) {
    node.second->update_match_index(id);
  }

  state_machine_interface_->on_elected();
}

CommandCommitResponseSharedFuture Context::complete_commit(
    CommandCommitResponseSharedPromise promise,
    CommandCommitResponseSharedFuture future, LogEntry::SharedPtr log,
    bool result, CommandCommitResponseCallback callback) {
  if (result == true) {
    store_->push_log(log);
    invoke_commit_callback(log);
  }

  auto response =
      CommandCommitResponse::make_shared(log->id_, log->command_, result);
  promise->set_value(response);
  if (callback != nullptr) {
    callback(future);
  }
  return future;
}

CommandCommitResponseSharedFuture Context::cancel_commit(
    CommandCommitResponseSharedPromise promise,
    CommandCommitResponseSharedFuture future, uint64_t id,
    CommandCommitResponseCallback callback) {
  auto response = CommandCommitResponse::make_shared(id, nullptr, false);
  promise->set_value(response);
  if (callback != nullptr) {
    callback(future);
  }
  return future;
}

CommandCommitResponseSharedFuture Context::commit_command(
    Command::SharedPtr command, CommandCommitResponseCallback callback) {
  CommandCommitResponseSharedPromise commit_promise =
      std::make_shared<CommandCommitResponsePromise>();
  CommandCommitResponseSharedFuture commit_future =
      commit_promise->get_future();
  if (state_machine_interface_->is_leader() == false) {
    return cancel_commit(commit_promise, commit_future, store_->logs_size(),
                         callback);
  }

  auto log = LogEntry::make_shared(store_->logs_size(), store_->current_term(),
                                   command);

  if (cluster_size_ <= 1) {
    return complete_commit(commit_promise, commit_future, log, true, callback);
  }

  if (set_pending_commit(std::make_shared<PendingCommit>(
          log, commit_promise, commit_future, callback)) == false) {
    return cancel_commit(commit_promise, commit_future, log->id_, callback);
  }

  return commit_future;
}

std::shared_ptr<PendingCommit> Context::get_pending_commit() {
  std::lock_guard<std::mutex> lock(pending_commit_mutex_);
  if (pending_commit_ == nullptr || pending_commit_->log_ == nullptr) {
    return nullptr;
  }

  if (pending_commit_->log_->id_ != store_->logs_size()) {
    pending_commit_ = nullptr;
    return nullptr;
  }

  return pending_commit_;
}

bool Context::set_pending_commit(std::shared_ptr<PendingCommit> commit) {
  if (commit->log_->id_ != store_->logs_size()) {
    return false;
  }

  std::lock_guard<std::mutex> lock(pending_commit_mutex_);

  if (pending_commit_ != nullptr && pending_commit_->log_ != nullptr) {
    if (pending_commit_->log_->id_ == store_->logs_size()) {
      return false;
    }
  }

  pending_commit_ = commit;

  return true;
}

void Context::cancel_pending_commit() {
  auto commit = clear_pending_commit();
  if (commit != nullptr && commit->log_ != nullptr) {
    complete_commit(commit->promise_, commit->future_, commit->log_, false,
                    commit->callback_);
  }
}

std::shared_ptr<PendingCommit> Context::clear_pending_commit() {
  std::shared_ptr<PendingCommit> commit;
  {
    std::lock_guard<std::mutex> lock(pending_commit_mutex_);
    commit = pending_commit_;
    pending_commit_ = nullptr;
  }
  return commit;
}

void Context::handle_pending_commit_response(const uint32_t id,
                                             const uint64_t commit_index,
                                             const uint64_t term,
                                             const bool success) {
  std::shared_ptr<PendingCommit> commit;
  bool result = false;

  {
    std::lock_guard<std::mutex> lock(pending_commit_mutex_);
    commit = pending_commit_;
    if (commit == nullptr || commit->log_ == nullptr ||
        commit->log_->id_ != commit_index || commit->log_->term_ != term) {
      return;
    }
    commit->result_map_[id] = success;

    unsigned int received_count = 1;
    unsigned int success_count = 1;
    for (auto &node : other_nodes_) {
      if (commit->result_map_.count(node.first) > 0) {
        received_count++;
        if (commit->result_map_[node.first] == success) {
          success_count++;
        }
      }
    }

    if (success_count < majority_) {
      return;
    }

    result = true;
    pending_commit_ = nullptr;
  }

  complete_commit(commit->promise_, commit->future_, commit->log_, result,
                  commit->callback_);
}

void Context::on_broadcast_response(const uint32_t id,
                                    const uint64_t commit_index,
                                    const uint64_t term, const bool success) {
  if (term >= store_->current_term()) {
    update_term(term);
  }

  handle_pending_commit_response(id, commit_index, term, success);
}

const std::shared_ptr<LogEntry> Context::on_log_get_request(uint64_t id) {
  auto commit = get_pending_commit();
  if (commit != nullptr && commit->log_ != nullptr && commit->log_->id_ == id) {
    return commit->log_;
  }

  return store_->log(id);
}

bool Context::is_valid_node(uint32_t id) { return other_nodes_.count(id) != 0; }

uint64_t Context::get_commands_size() { return store_->logs_size(); }

Command::SharedPtr Context::get_command(uint64_t id) {
  auto log = store_->log(id);
  if (log == nullptr) {
    return nullptr;
  }

  return log->command_;
}

void Context::register_on_committed(
    std::function<void(uint64_t, Command::SharedPtr)> callback) {
  set_commit_callback(callback);
}

void Context::set_commit_callback(
    std::function<void(uint64_t, Command::SharedPtr)> callback) {
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  commit_callback_ = callback;
}

void Context::register_on_reverted(std::function<void(uint64_t)> callback) {
  set_revert_callback(callback);
}

void Context::set_revert_callback(std::function<void(uint64_t)> callback) {
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  revert_callback_ = callback;
}

void Context::invoke_commit_callback(LogEntry::SharedPtr log) {
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  if (log != nullptr && commit_callback_ != nullptr) {
    commit_callback_(log->id_, log->command_);
  }
}

void Context::invoke_revert_callback(uint64_t id) {
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  if (revert_callback_ != nullptr) {
    revert_callback_(id);
  }
}

void Context::inspector_message_requested(
    foros_msgs::msg::Inspector::SharedPtr msg) {
  msg->stamp = node_clock_->get_clock()->now();
  msg->cluster_name = cluster_name_;
  msg->cluster_size = cluster_size_;
  msg->id = node_id_;
  msg->term = store_->current_term();
  msg->data_size = store_->logs_size();
  msg->voted_for = store_->voted_for();
  switch (state_machine_interface_->get_current_state()) {
    case StateType::kStandby:
      msg->state = foros_msgs::msg::Inspector::STANDBY;
      break;
    case StateType::kFollower:
      msg->state = foros_msgs::msg::Inspector::FOLLOWER;
      break;
    case StateType::kCandidate:
      msg->state = foros_msgs::msg::Inspector::CANDIDATE;
      break;
    case StateType::kLeader:
      msg->state = foros_msgs::msg::Inspector::LEADER;
      break;
    default:
      msg->state = foros_msgs::msg::Inspector::UNKNOWN;
      break;
  }
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
