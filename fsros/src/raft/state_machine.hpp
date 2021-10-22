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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_

#include <map>

#include "raft/event/event.hpp"
#include "raft/state/candidate.hpp"
#include "raft/state/follower.hpp"
#include "raft/state/leader.hpp"
#include "raft/state/standby.hpp"
#include "raft/state/state.hpp"
#include "raft/state/state_type.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

class StateMachine {
 public:
  void Handle(const Event &event);
  StateType GetCurrentState();

 private:
  std::map<StateType, State> states_ = {
      {StateType::kStandBy, Standby()},
      {StateType::kFollower, Follower()},
      {StateType::kCandidate, Candidate()},
      {StateType::kLeader, Leader()},
  };

  StateType current_state_ = StateType::kStandBy;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
