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

#ifndef AKIT_FAILSAFE_FSROS_COMMON_STATE_MACHINE_HPP_
#define AKIT_FAILSAFE_FSROS_COMMON_STATE_MACHINE_HPP_

#include <iostream>
#include <map>
#include <memory>

#include "common/observable.hpp"
#include "common/observer.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace common {

template <typename State, typename StateType, typename Event>
class StateMachine : public Observer<Event> {
 public:
  StateMachine(StateType current_state,
               std::map<StateType, std::shared_ptr<State>> states)
      : states_(states), current_state_(current_state) {
    // create event source and attach it to states
    event_notifier_ = std::make_shared<Observable<Event>>();
    event_notifier_->Subscribe(this);
    for (auto state = states_.begin(); state != states_.end(); state++) {
      state->second->SetEventNotifier(event_notifier_);
    }

    // invoke Entry() of initial state
    states_[current_state_]->Entry();
  }
  virtual ~StateMachine() { event_notifier_->Unsubscribe(this); }

  StateType GetCurrentState() { return current_state_; }

  void Handle(const Event &event) override {
    auto next_state = states_[current_state_]->Handle(event);
    if (states_.count(next_state) < 1) {
      std::cerr << "Invalid next state (" << static_cast<int>(next_state)
                << std::endl;
      return;
    }

    states_[current_state_]->Exit();
    current_state_ = next_state;
    states_[current_state_]->Entry();
    current_state_notifier_.Notify(current_state_);
  }

  void Subscribe(Observer<StateType> *observer) {
    current_state_notifier_.Subscribe(observer);
  }

  void Unsubscribe(Observer<StateType> *observer) {
    current_state_notifier_.Unsubscribe(observer);
  }

 private:
  std::map<StateType, std::shared_ptr<State>> states_;
  StateType current_state_;
  Observable<StateType> current_state_notifier_;
  std::shared_ptr<Observable<Event>> event_notifier_;
};

}  // namespace common
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_COMMON_STATE_MACHINE_HPP_
