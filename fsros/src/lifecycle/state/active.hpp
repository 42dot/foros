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

#ifndef AKIT_FAILSAFE_FSROS_LIFECYCLE_STATE_ACTIVE_HPP_
#define AKIT_FAILSAFE_FSROS_LIFECYCLE_STATE_ACTIVE_HPP_

#include <iostream>
#include <memory>

#include "lifecycle/event.hpp"
#include "lifecycle/state.hpp"

namespace akit {
namespace failsafe {
namespace fsros {
namespace lifecycle {

class Active final : public State {
 public:
  Active()
      : State(StateType::kActive,
              {{Event::kStandby, StateType::kStandby},
               {Event::kDeactivate, StateType::kInactive}}) {}

  void OnActivated() override;
  void OnDeactivated() override;
  void OnStandby() override;

  void Entry() override;
  void Exit() override;
};

}  // namespace lifecycle
}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_LIFECYCLE_STATE_ACTIVE_HPP_
