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
#include <memory>

#include "common/observer.hpp"
#include "common/state_machine.hpp"
#include "raft/event.hpp"
#include "raft/state.hpp"
#include "raft/state/candidate.hpp"
#include "raft/state/follower.hpp"
#include "raft/state/leader.hpp"
#include "raft/state/standby.hpp"
#include "raft/state_type.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

namespace common = akit::failsafe::fsros::common;

class StateMachine : public common::StateMachine<State, StateType, Event> {
 public:
  StateMachine()
      : common::StateMachine<State, StateType, Event>(
            StateType::kStandby,
            {{StateType::kStandby, std::make_shared<Standby>()},
             {StateType::kFollower, std::make_shared<Follower>()},
             {StateType::kCandidate, std::make_shared<Candidate>()},
             {StateType::kLeader, std::make_shared<Leader>()}}) {}
};

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
