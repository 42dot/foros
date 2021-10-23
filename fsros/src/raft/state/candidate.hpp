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

#include <memory>

#include "raft/event.hpp"
#include "raft/state.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

class Candidate final : public State {
 public:
  Candidate()
      : State(StateType::kCandidate,
              {{Event::kTerminated, StateType::kStandby},
               {Event::kTimedout, StateType::kCandidate},
               {Event::kElected, StateType::kLeader},
               {Event::kLeaderDiscovered, StateType::kFollower}}) {}

  void OnStarted() override;
  void OnTimedout() override;
  void OnVoteReceived() override;
  void OnLeaderDiscovered() override;
  void OnElected() override;
  void OnTerminated() override;

  void Entry() override;
  void Exit() override;
};

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_CANDIDATE_HPP_
