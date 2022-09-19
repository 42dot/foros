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

#include <rclcpp/logging.hpp>

#include <iostream>
#include <map>
#include <memory>

#include "common/observable.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace lifecycle {

State::State(StateType type, std::map<Event, StateType> transition_map,
             rclcpp::Logger &logger)
    : type_(type),
      transition_map_(transition_map),
      logger_(logger.get_child("lifecycle")) {}

StateType State::get_type() { return type_; }

void State::emit(const Event &event) { event_source_->notify(event); }

StateType State::handle(const Event &event) {
  if (transition_map_.count(event) < 1) {
    return StateType::kUnknown;
  }

  switch (event) {
    case Event::kActivate:
      on_activated();
      break;
    case Event::kDeactivate:
      on_deactivated();
      break;
    case Event::kStandby:
      on_standby();
      break;
    default:
      RCLCPP_ERROR(logger_, "Invalid event: %d", static_cast<int>(event));
      return type_;
  }

  return transition_map_[event];
}

void State::set_event_notifier(
    std::shared_ptr<Observable<Event>> event_source) {
  set_event_source(event_source);
}

void State::set_event_source(std::shared_ptr<Observable<Event>> event_source) {
  event_source_ = event_source;
}

}  // namespace lifecycle
}  // namespace foros
}  // namespace failover
}  // namespace akit
