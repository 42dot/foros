
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

#include "raft/raft_state_machine.hpp"

#include <iostream>

namespace akit {
namespace failsafe {
namespace fsros {

RaftState RaftStateMachine::GetCurrentState() {
  auto state_index = GetCurrentStateIndex();
  int min = static_cast<int>(RaftState::kStandBy);
  int max = static_cast<int>(RaftState::kUnknown);

  if (state_index < min || state_index > max) return RaftState::kUnknown;

  return static_cast<RaftState>(state_index);
}

void RaftStateMachine::Emit(RaftEvent event) {
  switch (event) {
    case RaftEvent::kStarted:
      Handle(started_event_);
      break;
    case RaftEvent::kTimedout:
      Handle(timedout_event_);
      break;
    case RaftEvent::kVoteReceived:
      Handle(vote_received_event_);
      break;
    case RaftEvent::kLeaderDiscovered:
      Handle(leader_discovered_event_);
      break;
    case RaftEvent::kElected:
      Handle(elected_event_);
      break;
    case RaftEvent::kTerminated:
      Handle(terminated_event_);
      break;
    default:
      std::cerr << "Unknow event notified" << std::endl;
      break;
  }
}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
