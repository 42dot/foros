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

#include "raft/state/standby.hpp"

#include <tuple>

namespace akit {
namespace failover {
namespace foros {
namespace raft {

void Standby::on_started() {}

void Standby::on_timedout() {}

void Standby::on_broadcast_timedout() {}

void Standby::on_leader_discovered() {}

void Standby::on_new_term_received() {}

void Standby::on_elected() {}

void Standby::on_terminated() {}

void Standby::entry() { context_->stop_election_timer(); }

void Standby::exit() {}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
