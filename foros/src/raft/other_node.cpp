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
    const std::string &cluster_name, const uint32_t node_id)
    : next_index_(0), match_index_(0) {
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

bool OtherNode::broadcast(uint64_t current_term, uint32_t node_id,
                          std::function<void(uint64_t, bool)> callback) {
  if (append_entries_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::AppendEntries::Request>();
  request->term = current_term;
  request->leader_id = node_id;

  send_append_entreis(request, callback);

  return true;
}

bool OtherNode::commit(uint64_t current_term, uint32_t node_id,
                       uint64_t prev_data_index, uint64_t prev_data_term,
                       Data::SharedPtr data,
                       std::function<void(uint64_t, bool)> callback) {
  if (append_entries_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::AppendEntries::Request>();
  request->term = current_term;
  request->leader_id = node_id;
  request->prev_data_index = prev_data_index;
  request->prev_data_term = prev_data_term;
  request->data = data->data_;

  send_append_entreis(request, callback);

  return true;
}

void OtherNode::send_append_entreis(
    foros_msgs::srv::AppendEntries::Request::SharedPtr request,
    std::function<void(uint64_t, bool)> callback) {
  auto response = append_entries_->async_send_request(
      request,
      [=](rclcpp::Client<
          foros_msgs::srv::AppendEntries>::SharedFutureWithRequest future) {
        auto ret = future.get();
        auto response = ret.second;
        callback(response->term, response->success);
      });
}

bool OtherNode::request_vote(uint64_t current_term, uint32_t node_id,
                             std::function<void(uint64_t, bool)> callback) {
  if (request_vote_->service_is_ready() == false) {
    return false;
  }

  auto request = std::make_shared<foros_msgs::srv::RequestVote::Request>();
  request->term = current_term;
  request->candidate_id = node_id;
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

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
