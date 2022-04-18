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

#ifndef AKIT_FAILOVER_FOROS_RAFT_STATE_MACHINE_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_STATE_MACHINE_HPP_

#include <rclcpp/logger.hpp>

#include <memory>
#include <vector>

#include "common/observer.hpp"
#include "common/state_machine.hpp"
#include "raft/context.hpp"
#include "raft/event.hpp"
#include "raft/state.hpp"
#include "raft/state/candidate.hpp"
#include "raft/state/follower.hpp"
#include "raft/state/leader.hpp"
#include "raft/state/standby.hpp"
#include "raft/state_machine_interface.hpp"
#include "raft/state_type.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

namespace common = akit::failover::foros::common;

class StateMachine : public common::StateMachine<State, StateType, Event>,
                     public StateMachineInterface {
 public:
  explicit StateMachine(const std::vector<uint32_t> &cluster_node_ids,
                        std::shared_ptr<Context> context,
                        rclcpp::Logger &logger);

 private:
  void on_election_timedout() override;
  void on_new_term_received() override;
  void on_elected() override;
  void on_broadcast_timedout() override;
  void on_leader_discovered() override;
  bool is_leader() override;
  StateType get_current_state() override;

  const std::shared_ptr<Context> context_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_STATE_MACHINE_HPP_
