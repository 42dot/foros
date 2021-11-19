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
    ClusterNodeDataInterface::SharedPtr data_interface)
    : node_id_(node_id),
      next_index_(next_index),
      match_index_(0),
      data_interface_(data_interface),
      data_replication_enabled_(data_interface_ != nullptr) {
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
                          const CommitInfo &last_commit,
                          std::function<void(const uint32_t, const uint64_t,
                                             const uint64_t, const bool)>
                              callback) {
  if (append_entries_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::AppendEntries::Request>();

  request->term = current_term;
  request->leader_id = node_id;

  if (data_replication_enabled_) {
    if (last_commit.index_ >= next_index_) {
      auto data = data_interface_->on_data_get_requested(next_index_);
      if (data != nullptr) {
        request->data = data->data();
        request->leader_commit = data->id();
        request->term = data->sub_id();
      }
    }

    auto data = data_interface_->on_data_get_requested(next_index_ - 1);
    if (data != nullptr) {
      request->prev_data_index = data->id();
      request->prev_data_term = data->sub_id();
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

        if (response->success) {
          this->match_index_ = request->leader_commit;
          this->next_index_ = this->match_index_ + 1;
        } else {
          if (this->next_index_ > 0) {
            this->next_index_--;
          }
        }

        callback(node_id_, request->leader_commit, response->term,
                 response->success);
      });
}

bool OtherNode::request_vote(
    const uint64_t current_term, const uint32_t node_id,
    const CommitInfo &last_commit,
    std::function<void(const uint64_t, const bool)> callback) {
  if (request_vote_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::RequestVote::Request>();
  request->term = current_term;
  request->candidate_id = node_id;
  request->last_data_index = last_commit.index_;
  request->loat_data_term = last_commit.term_;
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

void OtherNode::set_match_index(const uint64_t match_index) {
  match_index_ = match_index;
  next_index_ = match_index_ + 1;
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
