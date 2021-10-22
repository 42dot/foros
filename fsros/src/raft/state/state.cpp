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

#include "raft/state/state.hpp"

#include <iostream>
#include <map>

namespace akit {
namespace failsafe {
namespace fsros {

State::State() : type_(StateType::kUnknown) {}

State::State(StateType type, std::map<Event, StateType> transition_map)
    : type_(type), transition_map_(transition_map) {}

StateType State::GetType() { return type_; }

void State::OnStarted() {}

void State::OnTimedout() {}

void State::OnLeaderDiscovered() {}

void State::OnVoteReceived() {}

void State::OnElected() {}

void State::OnTerminated() {}

StateType State::Handle(const Event &event) {
  switch (event) {
    case Event::kStarted:
      OnStarted();
      break;
    case Event::kTimedout:
      OnTimedout();
      break;
    case Event::kLeaderDiscovered:
      OnLeaderDiscovered();
      break;
    case Event::kVoteReceived:
      OnVoteReceived();
      break;
    case Event::kElected:
      OnElected();
      break;
    case Event::kTerminated:
      OnTerminated();
      break;
    default:
      std::cerr << "State( << " << static_cast<int>(type_) << ") Event("
                << static_cast<int>(event) << " is not exist in callback map"
                << std::endl;
      return StateType::kUnknown;
  }

  if (transition_map_.count(event) < 1) {
    std::cerr << "State( << " << static_cast<int>(type_) << ") Event("
              << static_cast<int>(event) << " is not exist in transition map"
              << std::endl;
    return StateType::kUnknown;
  }

  return transition_map_[event];
}

void State::Entry() {}

void State::Exit() {}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
