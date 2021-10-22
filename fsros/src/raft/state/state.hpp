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

#ifndef AKIT_FAILSAFE_FSROS_RAFT_STATE_HPP_
#define AKIT_FAILSAFE_FSROS_RAFT_STATE_HPP_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "raft/event/event.hpp"
#include "raft/state/state_type.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

class State {
 public:
  State();
  State(StateType type, std::map<Event, StateType> transition_map);
  virtual ~State() {}

  StateType GetType();
  StateType Handle(const Event &event);

  virtual void OnStarted();
  virtual void OnTimedout();
  virtual void OnLeaderDiscovered();
  virtual void OnVoteReceived();
  virtual void OnElected();
  virtual void OnTerminated();

  virtual void Entry();
  virtual void Exit();

 private:
  StateType type_;
  std::map<Event, StateType> transition_map_;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_RAFT_STATE_HPP_
