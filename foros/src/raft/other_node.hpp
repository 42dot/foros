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

#ifndef AKIT_FAILOVER_FOROS_RAFT_OTHER_NODE_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_OTHER_NODE_HPP_

#include <foros_msgs/srv/append_entries.hpp>
#include <foros_msgs/srv/request_vote.hpp>
#include <rclcpp/client.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>

#include <functional>
#include <memory>
#include <string>

#include "raft/commit_info.hpp"
#include "raft/log_entry.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class OtherNode {
 public:
  OtherNode(
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
      const std::string &cluster_name, const uint32_t node_id,
      const uint64_t next_index,
      std::function<const std::shared_ptr<LogEntry>(uint64_t)>
          get_log_entry_callback);

  bool broadcast(const uint64_t current_term, const uint32_t node_id,
                 const LogEntry::SharedPtr log,
                 std::function<void(const uint32_t, const uint64_t,
                                    const uint64_t, const bool)>
                     callback);

  bool request_vote(const uint64_t current_term, const uint32_t node_id,
                    const LogEntry::SharedPtr log,
                    std::function<void(const uint64_t, const bool)> callback);

  void update_match_index(const uint64_t match_index);

 private:
  void send_append_entries(
      const foros_msgs::srv::AppendEntries::Request::SharedPtr request,
      std::function<void(const uint32_t, const uint64_t, const uint64_t,
                         const bool)>
          callback);
  void set_match_index(const uint64_t match_index);

  uint32_t node_id_;
  // index of the next log entry to send to this node
  uint64_t next_index_;
  // index of highest log entry known to be replicated on this node
  uint64_t match_index_;
  rclcpp::Client<foros_msgs::srv::AppendEntries>::SharedPtr append_entries_;
  rclcpp::Client<foros_msgs::srv::RequestVote>::SharedPtr request_vote_;
  std::function<const std::shared_ptr<LogEntry>(uint64_t)>
      get_log_entry_callback_;

  std::mutex index_mutex_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_OTHER_NODE_HPP_
