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
namespace raft {

void Standby::OnStarted() {}

void Standby::OnTimedout() {}

void Standby::OnVoteReceived() {}

void Standby::OnLeaderDiscovered() {}

void Standby::OnElected() {}

void Standby::OnTerminated() {}

void Standby::Entry() {}

void Standby::Exit() {}

}  // namespace raft
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
