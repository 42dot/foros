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

#include <memory>

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

StateMachine::StateMachine(
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services)
    : common::StateMachine<State, StateType, Event>(
          StateType::kStandby,
          {{StateType::kStandby, std::make_shared<Standby>()},
           {StateType::kFollower, std::make_shared<Follower>()},
           {StateType::kCandidate, std::make_shared<Candidate>()},
           {StateType::kLeader, std::make_shared<Leader>()}}),
      node_services_(node_services) {
  // TODO(wonguk.jeong): add raft services and clients
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
