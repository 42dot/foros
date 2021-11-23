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

#ifndef AKIT_FAILOVER_FOROS_LIFECYCLE_STATE_ACTIVE_HPP_
#define AKIT_FAILOVER_FOROS_LIFECYCLE_STATE_ACTIVE_HPP_

#include <iostream>
#include <memory>

#include "lifecycle/event.hpp"
#include "lifecycle/state.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace lifecycle {

class Active final : public State {
 public:
  explicit Active(rclcpp::Logger &logger)
      : State(StateType::kActive,
              {{Event::kStandby, StateType::kStandby},
               {Event::kDeactivate, StateType::kInactive}},
              logger) {}

  void on_activated() override;
  void on_deactivated() override;
  void on_standby() override;

  void entry() override;
  void exit() override;
};

}  // namespace lifecycle
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_LIFECYCLE_STATE_ACTIVE_HPP_
