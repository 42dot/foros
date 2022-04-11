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

#ifndef AKIT_FAILOVER_FOROS_COMMON_STATE_MACHINE_HPP_
#define AKIT_FAILOVER_FOROS_COMMON_STATE_MACHINE_HPP_

#include <iostream>
#include <map>
#include <memory>

#include "common/observable.hpp"
#include "common/observer.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace common {

template <typename State, typename StateType, typename Event>
class StateMachine : public Observer<Event> {
 public:
  StateMachine(StateType current_state,
               std::map<StateType, std::shared_ptr<State>> states)
      : states_(states), current_state_(current_state) {
    // create event source and attach it to states
    event_notifier_ = std::make_shared<Observable<Event>>();
    event_notifier_->subscribe(this);
    for (auto state = states_.begin(); state != states_.end(); state++) {
      state->second->set_event_notifier(event_notifier_);
    }

    // invoke Entry() of initial state
    states_[current_state_]->entry();
  }
  virtual ~StateMachine() { event_notifier_->unsubscribe(this); }

  StateType get_current_state_type() { return current_state_; }

  void handle(const Event &event) override {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    auto next_state = states_[current_state_]->handle(event);
    if (states_.count(next_state) < 1) {
      return;
    }

    states_[current_state_]->exit();
    current_state_ = next_state;
    current_state_notifier_.notify(current_state_);
    states_[current_state_]->entry();
  }

  void subscribe(Observer<StateType> *observer) {
    current_state_notifier_.subscribe(observer);
  }

  void unsubscribe(Observer<StateType> *observer) {
    current_state_notifier_.unsubscribe(observer);
  }

 private:
  std::map<StateType, std::shared_ptr<State>> states_;
  StateType current_state_;
  Observable<StateType> current_state_notifier_;
  std::shared_ptr<Observable<Event>> event_notifier_;
  std::recursive_mutex state_mutex_;
};

}  // namespace common
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_COMMON_STATE_MACHINE_HPP_
