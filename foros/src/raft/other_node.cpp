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

#include "raft/other_node.hpp"

#include <memory>
#include <string>

#include "common/node_util.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

OtherNode::OtherNode(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    const std::string &cluster_name, const uint32_t node_id,
    const uint64_t next_index,
    std::function<const std::shared_ptr<LogEntry>(uint64_t)>
        get_log_entry_callback)
    : node_id_(node_id),
      next_index_(next_index),
      match_index_(0),
      get_log_entry_callback_(get_log_entry_callback) {
  rcl_client_options_t options = rcl_client_get_default_options();
  options.qos = rmw_qos_profile_services_default;

  append_entries_ = rclcpp::Client<foros_msgs::srv::AppendEntries>::make_shared(
      node_base.get(), node_graph,
      NodeUtil::get_service_name(cluster_name, node_id,
                                 NodeUtil::kAppendEntriesServiceName),
      options);
  node_services->add_client(
      std::dynamic_pointer_cast<rclcpp::ClientBase>(append_entries_), nullptr);

  request_vote_ = rclcpp::Client<foros_msgs::srv::RequestVote>::make_shared(
      node_base.get(), node_graph,
      NodeUtil::get_service_name(cluster_name, node_id,
                                 NodeUtil::kRequestVoteServiceName),
      options);
  node_services->add_client(
      std::dynamic_pointer_cast<rclcpp::ClientBase>(request_vote_), nullptr);
}

bool OtherNode::broadcast(const uint64_t current_term, const uint32_t node_id,
                          const LogEntry::SharedPtr log,
                          std::function<void(const uint32_t, const uint64_t,
                                             const uint64_t, const bool)>
                              callback) {
  if (append_entries_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::AppendEntries::Request>();

  request->term = current_term;
  request->leader_id = node_id;

  uint64_t next_index;
  {
    std::lock_guard<std::mutex> lock(index_mutex_);
    next_index = next_index_;
  }

  if (get_log_entry_callback_ != nullptr) {
    if (log != nullptr && log->id_ >= next_index) {
      auto entry = get_log_entry_callback_(next_index);
      if (entry != nullptr) {
        request->entries = entry->command_->data();
        request->leader_commit = entry->id_;
        request->term = entry->term_;
      }
    }

    if (next_index > 0) {
      auto entry = get_log_entry_callback_(next_index - 1);
      if (entry != nullptr) {
        request->prev_log_index = entry->id_;
        request->prev_log_term = entry->term_;
      }
    }
  }

  send_append_entries(request, callback);

  return true;
}

void OtherNode::send_append_entries(
    const foros_msgs::srv::AppendEntries::Request::SharedPtr request,
    std::function<void(const uint32_t, const uint64_t, const uint64_t,
                       const bool)>
        callback) {
  auto response = append_entries_->async_send_request(
      request,
      [=](rclcpp::Client<
          foros_msgs::srv::AppendEntries>::SharedFutureWithRequest future) {
        auto ret = future.get();
        auto request = ret.first;
        auto response = ret.second;
        {
          std::lock_guard<std::mutex> lock(index_mutex_);
          if (response->success) {
            this->match_index_ = request->leader_commit;
            this->next_index_ = this->match_index_ + 1;
          } else {
            if (this->next_index_ > 0) {
              this->next_index_--;
            }
          }
        }
        callback(node_id_, request->leader_commit, response->term,
                 response->success);
      });
}

bool OtherNode::request_vote(
    const uint64_t current_term, const uint32_t node_id,
    const LogEntry::SharedPtr log,
    std::function<void(const uint64_t, const bool)> callback) {
  if (request_vote_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::RequestVote::Request>();
  request->term = current_term;
  request->candidate_id = node_id;
  request->last_data_index = log == nullptr ? 0 : log->id_;
  request->loat_data_term = log == nullptr ? 0 : log->term_;
  auto response = request_vote_->async_send_request(
      request,
      [=](rclcpp::Client<foros_msgs::srv::RequestVote>::SharedFutureWithRequest
              future) {
        auto ret = future.get();
        auto response = ret.second;
        callback(response->term, response->vote_granted);
      });

  return true;
}

void OtherNode::update_match_index(const uint64_t match_index) {
  set_match_index(match_index);
}

void OtherNode::set_match_index(const uint64_t match_index) {
  std::lock_guard<std::mutex> lock(index_mutex_);
  match_index_ = match_index;
  next_index_ = match_index_ + 1;
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
