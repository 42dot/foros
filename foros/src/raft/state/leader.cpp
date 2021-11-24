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

#include "raft/state/leader.hpp"

#include <tuple>

namespace akit {
namespace failover {
namespace foros {
namespace raft {

void Leader::on_started() {}

void Leader::on_timedout() {}

void Leader::on_broadcast_timedout() { context_->broadcast(); }

void Leader::on_leader_discovered() {}

void Leader::on_new_term_received() {}

void Leader::on_elected() {}

void Leader::on_terminated() {}

void Leader::entry() { context_->start_broadcast_timer(); }

void Leader::exit() {
  context_->stop_broadcast_timer();
  context_->cancel_pending_commit();
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
