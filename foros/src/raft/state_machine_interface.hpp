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

#ifndef AKIT_FAILOVER_FOROS_RAFT_STATE_MACHINE_ITERFACE_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_STATE_MACHINE_ITERFACE_HPP_

#include "raft/state_type.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class StateMachineInterface {
 public:
  virtual void on_election_timedout() = 0;
  virtual void on_new_term_received() = 0;
  virtual void on_elected() = 0;
  virtual void on_broadcast_timedout() = 0;
  virtual void on_leader_discovered() = 0;
  virtual bool is_leader() = 0;
  virtual StateType get_current_state() = 0;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_STATE_MACHINE_ITERFACE_HPP_
