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

namespace akit {
namespace failsafe {
namespace fsros {

StateTransitionTo<Follower> Standby::handle(const Started &) { return {}; }

StateTransitionStay Standby::handle(const Terminated &) { return {}; }

StateTransitionStay Standby::handle(const Timedout &) { return {}; }

StateTransitionStay Standby::handle(const VoteReceived &) { return {}; }

StateTransitionStay Standby::handle(const Elected &) { return {}; }

StateTransitionStay Standby::handle(const LeaderDiscovered &) { return {}; }

void Standby::entry() {}

void Standby::exit() {}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
