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

#include "lifecycle/state.hpp"

#include <iostream>
#include <map>
#include <memory>

#include "common/observable.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace lifecycle {

State::State(StateType type, std::map<Event, StateType> transition_map)
    : type_(type), transition_map_(transition_map) {}

StateType State::GetType() { return type_; }

void State::Emit(const Event &event) { event_source_->Notify(event); }

StateType State::Handle(const Event &event) {
  if (transition_map_.count(event) < 1) {
    return type_;
  }

  switch (event) {
    case Event::kActivate:
      OnActivated();
      break;
    case Event::kDeactivate:
      OnDeactivated();
      break;
    case Event::kStandby:
      OnStandby();
      break;
    default:
      std::cerr << "Invalid event: " << static_cast<int>(event) << std::endl;
      return type_;
  }

  return transition_map_[event];
}

void State::SetEventSource(std::shared_ptr<Observable<Event>> event_source) {
  event_source_ = event_source;
}

}  // namespace lifecycle
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
