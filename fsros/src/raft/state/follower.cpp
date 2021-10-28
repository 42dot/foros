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

#include "raft/state/follower.hpp"

#include <rclcpp/duration.hpp>
#include <rclcpp/timer.hpp>

#include <chrono>
#include <functional>
#include <iostream>
#include <tuple>

#include "raft/event.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

void Follower::on_started() {}

void Follower::on_timedout() {
  std::cerr << "Follower: timedout event" << std::endl;
}

void Follower::on_vote_received() {}

void Follower::on_leader_discovered() {}

void Follower::on_elected() {}

void Follower::on_terminated() {}

std::tuple<uint64_t, bool> Follower::on_append_entries_received(uint64_t term) {
  // TODO(wonguk.jeong): Log replicateion is not implemented yet
  bool success = false;
  if (term < context_->current_term_) {
    std::cerr << "Follower: new term (" << term
              << ") is less than existing one (" << context_->current_term_
              << ")" << std::endl;

  } else {
    std::cout << "Follower: restart election timer since we received new term ("
              << term << ")" << std::endl;
    context_->current_term_ = term;
    success = true;
  }

  return std::make_tuple(context_->current_term_, success);
}

void Follower::entry() { context_->start_election_timer(); }

void Follower::exit() {}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
