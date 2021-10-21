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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_STATE_CANDIDATE_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_STATE_CANDIDATE_HPP_

#include "raft/event/elected.hpp"
#include "raft/event/leadaer_discovered.hpp"
#include "raft/event/started.hpp"
#include "raft/event/terminated.hpp"
#include "raft/event/timedout.hpp"
#include "raft/event/vote_received.hpp"
#include "raft/state/state.hpp"
#include "raft/state_transition.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

class Follower;
class Leader;
class Standby;

class Candidate final : public State {
 public:
  StateTransitionStay handle(const Started &event);
  StateTransitionTo<Standby> handle(const Terminated &event);
  StateTransitionTo<Candidate> handle(const Timedout &event);
  StateTransitionStay handle(const VoteReceived &event);
  StateTransitionTo<Leader> handle(const Elected &event);
  StateTransitionTo<Follower> handle(const LeaderDiscovered &event);

  void entry() override;
  void exit() override;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_CANDIDATE_HPP_
