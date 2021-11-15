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

#ifndef AKIT_FAILOVER_FOROS_RAFT_CONTEXT_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_CONTEXT_HPP_

#include <foros_msgs/srv/append_entries.hpp>
#include <foros_msgs/srv/request_vote.hpp>
#include <rclcpp/any_service_callback.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/timer.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "akit/failover/foros/cluster_node_data_interface.hpp"
#include "akit/failover/foros/data.hpp"
#include "raft/commit_info.hpp"
#include "raft/other_node.hpp"
#include "raft/state_machine_interface.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class Context {
 public:
  Context(
      const std::string &cluster_name, const uint32_t node_id,
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
      rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
      rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
      unsigned int election_timeout_min, unsigned int election_timeout_max,
      ClusterNodeDataInterface &data_interface);

  void initialize(const std::vector<uint32_t> &cluster_node_ids,
                  StateMachineInterface *state_machine_interface);
  void start_election_timer();
  void stop_election_timer();
  void reset_election_timer();
  void start_broadcast_timer();
  void stop_broadcast_timer();
  void reset_broadcast_timer();
  std::string get_node_name();
  void vote_for_me();
  std::tuple<uint64_t, bool> vote(uint64_t term, uint32_t id);
  void reset_vote();
  void increase_term();
  uint64_t get_term();
  void broadcast();
  void request_vote();
  DataCommitResponseSharedFuture commit_data(
      Data::SharedPtr data, DataCommitResponseCallback callback);
  uint64_t get_data_commit_index();

 private:
  void initialize_node();
  void initialize_other_nodes(const std::vector<uint32_t> &cluster_node_ids);
  void on_append_entries_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<foros_msgs::srv::AppendEntries::Request> request,
      std::shared_ptr<foros_msgs::srv::AppendEntries::Response> response);
  void on_request_vote_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<foros_msgs::srv::RequestVote::Request> request,
      std::shared_ptr<foros_msgs::srv::RequestVote::Response> response);
  bool update_term(uint64_t term);
  void on_request_vote_response(uint64_t term, bool vote_granted);
  void check_elected();

  uint32_t request_commit(Data::SharedPtr data);
  void on_commit_response(uint64_t term, bool success);

  const std::string cluster_name_;
  uint32_t node_id_;

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph_;
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services_;
  rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers_;
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock_;

  std::shared_ptr<rclcpp::Service<foros_msgs::srv::AppendEntries>>
      append_entries_service_;
  rclcpp::AnyServiceCallback<foros_msgs::srv::AppendEntries>
      append_entries_callback_;
  std::shared_ptr<rclcpp::Service<foros_msgs::srv::RequestVote>>
      request_vote_service_;
  rclcpp::AnyServiceCallback<foros_msgs::srv::RequestVote>
      request_vote_callback_;

  std::vector<std::shared_ptr<OtherNode>> other_nodes_ = {};

  // Essential fields for RAFT
  uint64_t current_term_;  // latest election term
  uint32_t voted_for_;  // candidate node id that received vote in current term
  bool voted_;          // flag to check whether voted in current term or not
  unsigned int vote_received_;  // number of received votes in current term
  unsigned int available_candidates_;  // number of available candidate

  CommitInfo last_commit_;  // index of highest data entry known to be committed
  CommitInfo last_applied_;  // index of highest data entry applied to fsm

  unsigned int election_timeout_min_;  // minimum election timeout in msecs
  unsigned int election_timeout_max_;  // maximum election timeout in msecs
  std::random_device random_device_;   // random seed for election timeout
  std::mt19937 random_generator_;      // random generator for election timeout
  rclcpp::TimerBase::SharedPtr election_timer_;  // election timeout timer

  unsigned int broadcast_timeout_;                // broadcast timeout
  rclcpp::TimerBase::SharedPtr broadcast_timer_;  // broadcast timer
  bool broadcast_received_;  // flag to check whether boradcast recevied before
                             // election timer expired

  std::map<
      int64_t,
      std::tuple<DataCommitResponseSharedPromise, DataCommitResponseCallback,
                 DataCommitResponseSharedFuture, std::shared_ptr<CommitInfo>>>
      pending_commits_;
  std::mutex pending_commits_mutex_;

  StateMachineInterface *state_machine_interface_;
  ClusterNodeDataInterface &data_interface_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_CONTEXT_HPP_
