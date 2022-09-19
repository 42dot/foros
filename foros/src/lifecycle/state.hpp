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

#ifndef AKIT_FAILOVER_FOROS_LIFECYCLE_STATE_HPP_
#define AKIT_FAILOVER_FOROS_LIFECYCLE_STATE_HPP_

#include <rclcpp/logger.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "common/observable.hpp"
#include "lifecycle/event.hpp"
#include "lifecycle/state_type.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace lifecycle {

class State {
 public:
  State(StateType type, std::map<Event, StateType> transition_map,
        rclcpp::Logger &logger);
  virtual ~State() {}

  StateType get_type();
  StateType handle(const Event &event);
  void emit(const Event &event);
  void set_event_notifier(std::shared_ptr<Observable<Event>> event_source);

  virtual void on_activated() = 0;
  virtual void on_deactivated() = 0;
  virtual void on_standby() = 0;

  virtual void entry() = 0;
  virtual void exit() = 0;

 private:
  void set_event_source(std::shared_ptr<Observable<Event>> event_source);

  StateType type_;
  std::shared_ptr<Observable<Event>> event_source_;
  std::map<Event, StateType> transition_map_;
  rclcpp::Logger logger_;
};

}  // namespace lifecycle
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_LIFECYCLE_STATE_HPP_
