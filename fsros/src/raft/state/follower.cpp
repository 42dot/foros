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

#include "raft/event.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

void Follower::on_started() {}

void Follower::on_election_timer_expired() {
  std::cerr << "Follower: emit timedout event" << std::endl;
  emit(Event::kTimedout);
}

void Follower::on_timedout() {}

void Follower::on_vote_received() {}

void Follower::on_leader_discovered() {}

void Follower::on_elected() {}

void Follower::on_terminated() {}

void Follower::on_append_entries_received(uint64_t term) {
  if (term <= term_) {
    std::cerr << "Follower: new term (" << term
              << ") is not greater than existing one (" << term_ << ")"
              << std::endl;
    return;
  }

  std::cout << "Follower: restart election timer since we received new term ("
            << term << ")" << std::endl;
  term_ = term;
  start_election_timer();
}

void Follower::entry() { start_election_timer(); }

void Follower::exit() { stop_election_timer(); }

void Follower::start_election_timer() {
  stop_election_timer();
  timer_ = context_->create_election_timer(
      std::bind(&Follower::on_election_timer_expired, this));
}

void Follower::stop_election_timer() {
  if (timer_ != nullptr) {
    timer_->cancel();
    timer_.reset();
  }
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
