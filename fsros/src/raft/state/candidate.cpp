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

#include "raft/state/candidate.hpp"

#include <tuple>

namespace akit {
namespace failsafe {
namespace fsros {
namespace raft {

void Candidate::on_started() {}

void Candidate::on_timedout() { start_election(); }

void Candidate::on_vote_received() {}

void Candidate::on_leader_discovered() {}

void Candidate::on_new_term_received() {}

void Candidate::on_elected() {}

void Candidate::on_terminated() {}

void Candidate::entry() { start_election(); }

void Candidate::exit() {}

void Candidate::start_election() {
  context_->increase_term();
  context_->vote_for_me();
  context_->reset_election_timer();
  // TODO(wonguk.jeong): send RequestVote RPCs to others
  // TODO(wonguk.jeong): check available nodes
}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
