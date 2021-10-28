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

#include "common/observable.hpp"
#include "raft/context.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

State::State(StateType type, std::map<Event, StateType> transition_map,
             std::shared_ptr<Context> context)
    : context_(context), type_(type), transition_map_(transition_map) {}

StateType State::get_type() { return type_; }

void State::emit(const Event &event) { event_notifier_->notify(event); }

StateType State::handle(const Event &event) {
  if (transition_map_.count(event) < 1) {
    return type_;
  }

  switch (event) {
    case Event::kStarted:
      on_started();
      break;
    case Event::kTimedout:
      on_timedout();
      break;
    case Event::kLeaderDiscovered:
      on_leader_discovered();
      break;
    case Event::kNewTermReceived:
      on_new_term_received();
      break;
    case Event::kVoteReceived:
      on_vote_received();
      break;
    case Event::kElected:
      on_elected();
      break;
    case Event::kTerminated:
      on_terminated();
      break;
    default:
      std::cerr << "[" << static_cast<int>(type_)
                << "]: invalid event: " << static_cast<int>(event) << std::endl;
      return type_;
  }

  return transition_map_[event];
}

void State::set_event_notifier(
    std::shared_ptr<Observable<Event>> event_source) {
  event_notifier_ = event_source;
}

bool State::update_term(uint64_t term) {
  if (term <= context_->current_term_) return false;

  std::cout << "[" << static_cast<int>(type_)
            << "]: new term received, emit event." << std::endl;
  context_->current_term_ = term;

  emit(Event::kNewTermReceived);

  return true;
}

std::tuple<uint64_t, bool> State::on_append_entries_received(uint64_t term) {
  bool success = true;

  if (term < context_->current_term_) {
    std::cerr << "[" << static_cast<int>(type_) << "]: new term (" << term
              << ") is less than existing one (" << context_->current_term_
              << ")" << std::endl;
    success = false;
  } else {
    update_term(term);
    success = true;
  }

  return std::make_tuple(context_->current_term_, success);
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
