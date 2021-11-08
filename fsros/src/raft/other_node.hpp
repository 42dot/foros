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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_OTHER_NODE_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_OTHER_NODE_HPP_

#include <fsros_msgs/srv/append_entries.hpp>
#include <fsros_msgs/srv/request_vote.hpp>
#include <rclcpp/client.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>

#include <functional>
#include <memory>
#include <string>

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

class OtherNode {
 public:
  OtherNode(
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
      const std::string& cluster_name, const uint32_t node_id);

  bool broadcast(uint64_t current_term, uint32_t node_id,
                 std::function<void(uint64_t)> callback);

  bool request_vote(uint64_t current_term, uint32_t node_id,
                    std::function<void(uint64_t, bool)> callback);

 private:
  // index of highest log entry known to be commited
  uint64_t commit_index_;
  // index of highest log entry applied to state machine
  uint64_t last_applied_;
  rclcpp::Client<fsros_msgs::srv::AppendEntries>::SharedPtr append_entries_;
  rclcpp::Client<fsros_msgs::srv::RequestVote>::SharedPtr request_vote_;
};

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_OTHER_NODE_HPP_
