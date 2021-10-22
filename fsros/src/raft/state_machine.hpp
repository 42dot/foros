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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_

#include <map>
#include <memory>

#include "raft/event/event.hpp"
#include "raft/event/event_listener.hpp"
#include "raft/event/event_observer.hpp"
#include "raft/state/state.hpp"
#include "raft/state/state_type.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

class StateMachine : public EventListener {
 public:
  StateMachine();

  StateType GetCurrentState();
  void Handle(const Event &event);
  void OnEventReceived(const Event &event) override;

 private:
  std::map<StateType, std::shared_ptr<State>> states_ = {};
  StateType current_state_ = StateType::kStandBy;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_MACHINE_HPP_
