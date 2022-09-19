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

#include "raft/state.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <tuple>

#include "common/observable.hpp"
#include "raft/context.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

State::State(StateType type, std::map<Event, StateType> transition_map,
             std::shared_ptr<Context> context, rclcpp::Logger &logger)
    : context_(context),
      type_(type),
      transition_map_(transition_map),
      logger_(logger.get_child("raft")) {
  callback_map_ = {
      {Event::kStarted, std::bind(&State::on_started, this)},
      {Event::kTimedout, std::bind(&State::on_timedout, this)},
      {Event::kLeaderDiscovered, std::bind(&State::on_leader_discovered, this)},
      {Event::kNewTermReceived, std::bind(&State::on_new_term_received, this)},
      {Event::kElected, std::bind(&State::on_elected, this)},
      {Event::kTerminated, std::bind(&State::on_terminated, this)},
      {Event::kBroadcastTimedout,
       std::bind(&State::on_broadcast_timedout, this)},
  };
}

StateType State::get_type() { return type_; }

void State::emit(const Event &event) { event_notifier_->notify(event); }

StateType State::handle(const Event &event) {
  if (transition_map_.count(event) < 1) {
    return StateType::kStay;
  }

  if (callback_map_.count(event) < 1) {
    RCLCPP_ERROR(logger_, "[%d]: invalid event: %d", static_cast<int>(type_),
                 static_cast<int>(event));
    return type_;
  }

  callback_map_[event]();

  return transition_map_[event];
}

void State::set_event_notifier(
    std::shared_ptr<Observable<Event>> event_source) {
  set_event_source(event_source);
}

void State::set_event_source(std::shared_ptr<Observable<Event>> event_source) {
  event_notifier_ = event_source;
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
