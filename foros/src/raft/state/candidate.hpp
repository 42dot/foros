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

#ifndef AKIT_FAILOVER_FOROS_RAFT_STATE_CANDIDATE_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_STATE_CANDIDATE_HPP_

#include <rclcpp/timer.hpp>

#include <memory>
#include <tuple>

#include "raft/context.hpp"
#include "raft/event.hpp"
#include "raft/state.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class Candidate final : public State {
 public:
  explicit Candidate(std::shared_ptr<Context> context, rclcpp::Logger &logger)
      : State(StateType::kCandidate,
              {{Event::kTerminated, StateType::kStandby},
               {Event::kTimedout, StateType::kStay},
               {Event::kElected, StateType::kLeader},
               {Event::kLeaderDiscovered, StateType::kFollower},
               {Event::kNewTermReceived, StateType::kFollower}},
              context, logger) {}

  void on_started() override;
  void on_timedout() override;
  void on_broadcast_timedout() override;
  void on_leader_discovered() override;
  void on_new_term_received() override;
  void on_elected() override;
  void on_terminated() override;

  void entry() override;
  void exit() override;

 private:
  void start_election();
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_STATE_CANDIDATE_HPP_
