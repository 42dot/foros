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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_IMPL_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_IMPL_HPP_

#include <rclcpp/logger.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_options.hpp>

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "akit/failover/foros/cluster_node_options.hpp"
#include "akit/failover/foros/command.hpp"
#include "common/observer.hpp"
#include "lifecycle/state_machine.hpp"
#include "lifecycle/state_type.hpp"
#include "raft/context.hpp"
#include "raft/state_machine.hpp"

namespace akit {
namespace failover {
namespace foros {

class ClusterNodeImpl final : Observer<lifecycle::StateType>,
                              Observer<raft::StateType> {
 public:
  explicit ClusterNodeImpl(
      const std::string &cluster_name, const uint32_t node_id,
      const std::vector<uint32_t> &cluster_node_ids,
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
      rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
      rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging,
      rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
      rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
      rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
      rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
      const ClusterNodeOptions &options);

  ~ClusterNodeImpl();

  void handle(const lifecycle::StateType &state) override;
  void handle(const raft::StateType &state) override;
  bool is_activated();
  void register_on_activated(std::function<void()> callback);
  void register_on_deactivated(std::function<void()> callback);
  void register_on_standby(std::function<void()> callback);
  CommandCommitResponseSharedFuture commit_command(
      Command::SharedPtr command, CommandCommitResponseCallback &callback);
  uint64_t get_commands_size();
  Command::SharedPtr get_command(uint64_t id);
  void register_on_committed(
      std::function<void(const uint64_t, Command::SharedPtr)> callback);
  void register_on_reverted(std::function<void(const uint64_t)> callback);

 private:
  void set_activated_callback(std::function<void()> callback);
  void set_deactivated_callback(std::function<void()> callback);
  void set_standby_callback(std::function<void()> callback);

  rclcpp::Logger logger_;
  std::shared_ptr<raft::Context> raft_context_;
  std::unique_ptr<raft::StateMachine> raft_fsm_;
  std::unique_ptr<lifecycle::StateMachine> lifecycle_fsm_;
  std::function<void()> activated_callback_;
  std::function<void()> deactivated_callback_;
  std::function<void()> standby_callback_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_IMPL_HPP_
