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

namespace akit {
namespace failsafe {
namespace fsros {

StateTransitionStay Leader::handle(const Started &) { return {}; }

StateTransitionTo<Standby> Leader::handle(const Terminated &) { return {}; }

StateTransitionStay Leader::handle(const Timedout &) { return {}; }

StateTransitionStay Leader::handle(const VoteReceived &) { return {}; }

StateTransitionStay Leader::handle(const Elected &) { return {}; }

StateTransitionTo<Follower> Leader::handle(const LeaderDiscovered &) {
  return {};
}

void Leader::entry() {}

void Leader::exit() {}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
