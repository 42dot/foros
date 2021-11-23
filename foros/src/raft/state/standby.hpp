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

#ifndef AKIT_FAILOVER_FOROS_RAFT_STATE_STANDBY_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_STATE_STANDBY_HPP_

#include <iostream>
#include <memory>
#include <tuple>

#include "raft/context.hpp"
#include "raft/event.hpp"
#include "raft/state.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class Standby final : public State {
 public:
  explicit Standby(std::shared_ptr<Context> context, rclcpp::Logger &logger)
      : State(StateType::kStandby, {{Event::kStarted, StateType::kFollower}},
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
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_STATE_STANDBY_HPP_
