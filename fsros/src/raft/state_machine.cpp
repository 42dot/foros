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

#include "raft/event/event_observer.hpp"
#include "raft/state/candidate.hpp"
#include "raft/state/follower.hpp"
#include "raft/state/leader.hpp"
#include "raft/state/standby.hpp"
#include "raft/state/state.hpp"
#include "raft/state/state_type.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

StateMachine::StateMachine() {
  auto observer = std::make_shared<EventObserver>(this);
  states_[StateType::kStandBy] = std::make_shared<Standby>(observer);
  states_[StateType::kFollower] = std::make_shared<Follower>(observer);
  states_[StateType::kCandidate] = std::make_shared<Candidate>(observer);
  states_[StateType::kLeader] = std::make_shared<Leader>(observer);

  // Invoke "Entry" of initial state
  states_[current_state_]->Entry();
}

void StateMachine::Handle(const Event &event) {
  auto next_state = states_[current_state_]->Handle(event);
  if (states_.count(next_state) < 1) {
    std::cerr << "Invalid next state (" << static_cast<int>(next_state)
              << std::endl;
    return;
  }

  states_[current_state_]->Exit();
  current_state_ = next_state;
  states_[current_state_]->Entry();
}

void StateMachine::OnEventReceived(const Event &event) { Handle(event); }

StateType StateMachine::GetCurrentState() { return current_state_; }

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
