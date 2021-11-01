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

#include "raft/state_machine.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include "raft/context.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

StateMachine::StateMachine(const std::vector<uint32_t> &cluster_node_ids,
                           std::shared_ptr<Context> context)
    : common::StateMachine<State, StateType, Event>(
          StateType::kStandby,
          {{StateType::kStandby, std::make_shared<Standby>(context)},
           {StateType::kFollower, std::make_shared<Follower>(context)},
           {StateType::kCandidate, std::make_shared<Candidate>(context)},
           {StateType::kLeader, std::make_shared<Leader>(context)}}),
      context_(context) {
  context_->initialize(cluster_node_ids, this);
}

void StateMachine::on_election_timedout() { handle(Event::kTimedout); }

void StateMachine::on_new_term_received() { handle(Event::kNewTermReceived); }

void StateMachine::on_elected() { handle(Event::kElected); }

void StateMachine::on_broadcast_timedout() {
  handle(Event::kBroadcastTimedout);
}

void StateMachine::on_leader_discovered() { handle(Event::kLeaderDiscovered); }

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
