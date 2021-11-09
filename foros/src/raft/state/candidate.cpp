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
namespace failover {
namespace foros {
namespace raft {

void Candidate::on_started() {}

void Candidate::on_timedout() { start_election(); }

void Candidate::on_broadcast_timedout() {}

void Candidate::on_leader_discovered() {}

void Candidate::on_new_term_received() {}

void Candidate::on_elected() {}

void Candidate::on_terminated() {}

void Candidate::entry() { start_election(); }

void Candidate::exit() {}

void Candidate::start_election() {
  context_->increase_term();
  context_->reset_vote();
  context_->vote_for_me();
  context_->reset_election_timer();
  context_->request_vote();
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
