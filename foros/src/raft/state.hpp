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

#ifndef AKIT_FAILOVER_FOROS_RAFT_STATE_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_STATE_HPP_

#include <rclcpp/logger.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>

#include "common/observable.hpp"
#include "raft/context.hpp"
#include "raft/event.hpp"
#include "raft/state_type.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class State {
 public:
  State(StateType type, std::map<Event, StateType> transition_map,
        std::shared_ptr<Context> context, rclcpp::Logger &logger);
  virtual ~State() {}

  StateType get_type();
  StateType handle(const Event &event);
  void emit(const Event &event);
  void set_event_notifier(std::shared_ptr<Observable<Event>> event_source);

  virtual void on_started() = 0;
  virtual void on_timedout() = 0;
  virtual void on_broadcast_timedout() = 0;
  virtual void on_leader_discovered() = 0;
  virtual void on_new_term_received() = 0;
  virtual void on_elected() = 0;
  virtual void on_terminated() = 0;

  virtual void entry() = 0;
  virtual void exit() = 0;

 protected:
  std::shared_ptr<Context> context_;

 private:
  void set_event_source(std::shared_ptr<Observable<Event>> event_source);

  StateType type_;
  std::shared_ptr<Observable<Event>> event_notifier_;
  std::map<Event, StateType> transition_map_;
  std::map<Event, std::function<void()>> callback_map_;
  rclcpp::Logger logger_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_STATE_HPP_
