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

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

void Follower::on_started() {}

void Follower::on_election_timer_expired() {
  // FIXME: temporary log for test
  std::cerr << "timedout" << std::endl;
}

void Follower::on_timedout() {}

void Follower::on_vote_received() {}

void Follower::on_leader_discovered() {}

void Follower::on_elected() {}

void Follower::on_terminated() {}

void Follower::entry() {
  timer_ = context_->create_election_timer(
      std::bind(&Follower::on_election_timer_expired, this));
}

void Follower::exit() {
  timer_->cancel();
  timer_.reset();
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
