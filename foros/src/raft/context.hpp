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
#include <rclcpp/logger.hpp>
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

#include "akit/failover/foros/command.hpp"
#include "raft/commit_info.hpp"
#include "raft/context_store.hpp"
#include "raft/inspector.hpp"
#include "raft/other_node.hpp"
#include "raft/pending_commit.hpp"
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
      rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
      rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
      rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
      const unsigned int election_timeout_min,
      const unsigned int election_timeout_max,
      const std::string &temp_directory, rclcpp::Logger &logger);

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
  void reset_vote();
  void increase_term();
  uint64_t get_term();
  void broadcast();
  void request_vote();
  CommandCommitResponseSharedFuture commit_command(
      Command::SharedPtr command, CommandCommitResponseCallback callback);
  void cancel_pending_commit();
  uint64_t get_commands_size();
  Command::SharedPtr get_command(uint64_t id);
  void register_on_committed(
      std::function<void(const uint64_t, Command::SharedPtr)> callback);
  void register_on_reverted(std::function<void(const uint64_t)> callback);

 private:
  void initialize_node();
  void initialize_other_nodes(const std::vector<uint32_t> &cluster_node_ids);
  void set_cluster_size(uint32_t size);
  void set_state_machine_interface(
      StateMachineInterface *state_machine_interface);

  bool update_term(uint64_t term, bool self = false);
  bool is_valid_node(uint32_t id);

  void invoke_commit_callback(LogEntry::SharedPtr log);
  void invoke_revert_callback(uint64_t id);

  // Voting methods
  std::tuple<uint64_t, bool> vote(const uint64_t term, const uint32_t id,
                                  const uint64_t last_command_index,
                                  const uint64_t last_command_term);
  void on_request_vote_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<foros_msgs::srv::RequestVote::Request> request,
      std::shared_ptr<foros_msgs::srv::RequestVote::Response> response);
  void on_request_vote_response(const uint64_t term, const bool vote_granted);
  void check_elected();
  void set_voted_for(uint32_t id);

  // Data replication methods
  void on_append_entries_requested(
      const std::shared_ptr<rmw_request_id_t> header,
      const std::shared_ptr<foros_msgs::srv::AppendEntries::Request> request,
      std::shared_ptr<foros_msgs::srv::AppendEntries::Response> response);
  uint32_t request_remote_commit(const Command::SharedPtr command);
  bool request_local_commit(
      const std::shared_ptr<foros_msgs::srv::AppendEntries::Request> request);
  void request_local_rollback(const uint64_t commit_index);
  void on_broadcast_response(const uint32_t id, const uint64_t commit_index,
                             const uint64_t term, const bool success);
  CommandCommitResponseSharedFuture complete_commit(
      CommandCommitResponseSharedPromise promise,
      CommandCommitResponseSharedFuture future, LogEntry::SharedPtr log,
      bool result, CommandCommitResponseCallback callback);
  CommandCommitResponseSharedFuture cancel_commit(
      CommandCommitResponseSharedPromise promise,
      CommandCommitResponseSharedFuture future, uint64_t id,
      CommandCommitResponseCallback callback);
  std::shared_ptr<PendingCommit> get_pending_commit();
  bool set_pending_commit(std::shared_ptr<PendingCommit> commit);
  void handle_pending_commit_response(const uint32_t id,
                                      const uint64_t commit_index,
                                      const uint64_t term, const bool success);
  const std::shared_ptr<LogEntry> on_log_get_request(uint64_t id);
  void inspector_message_requested(foros_msgs::msg::Inspector::SharedPtr msg);

  std::shared_ptr<PendingCommit> clear_pending_commit();

  void set_commit_callback(
      std::function<void(uint64_t, Command::SharedPtr)> callback);
  void set_revert_callback(std::function<void(uint64_t)> callback);

  const std::string cluster_name_;
  uint32_t node_id_;

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph_;
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services_;
  rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers_;
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock_;

  rclcpp::Service<foros_msgs::srv::AppendEntries>::SharedPtr
      append_entries_service_;
  rclcpp::AnyServiceCallback<foros_msgs::srv::AppendEntries>
      append_entries_callback_;
  rclcpp::Service<foros_msgs::srv::RequestVote>::SharedPtr
      request_vote_service_;
  rclcpp::AnyServiceCallback<foros_msgs::srv::RequestVote>
      request_vote_callback_;

  std::map<uint32_t, std::shared_ptr<OtherNode>> other_nodes_;

  std::unique_ptr<ContextStore> store_;  // raft data store

  uint32_t majority_;                  // number of majority of the full cluster
  uint32_t cluster_size_;              // number of nodes in the cluster
  unsigned int election_timeout_min_;  // minimum election timeout in msecs
  unsigned int election_timeout_max_;  // maximum election timeout in msecs
  std::random_device random_device_;   // random seed for election timeout
  std::mt19937 random_generator_;      // random generator for election timeout
  rclcpp::TimerBase::SharedPtr election_timer_;  // election timeout timer

  unsigned int broadcast_timeout_;                // broadcast timeout
  rclcpp::TimerBase::SharedPtr broadcast_timer_;  // broadcast timer
  bool broadcast_received_;  // flag to check whether boradcast recevied
                             // before election timer expired

  std::mutex pending_commit_mutex_;
  std::shared_ptr<PendingCommit> pending_commit_;
  std::function<void(uint64_t, Command::SharedPtr)> commit_callback_;
  std::function<void(uint64_t)> revert_callback_;

  StateMachineInterface *state_machine_interface_;

  rclcpp::Logger logger_;

  std::recursive_mutex callback_mutex_;

  std::unique_ptr<Inspector> inspector_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_CONTEXT_HPP_
