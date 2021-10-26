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

#ifndef AKIT_FAILSAFE_FSROS_CLUSTER_NODE_INTERFACE_HPP_
#define AKIT_FAILSAFE_FSROS_CLUSTER_NODE_INTERFACE_HPP_

#include "akit/failsafe/fsros/common.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

enum class ClusterNodeState {
  kActive,
  kInactive,
  kStandby,
  kUnknown,
};

class ClusterNodeInterface {
 public:
  /// Callback function for activate transition
  CLUSTER_NODE_PUBLIC
  virtual void on_activated() = 0;

  /// Callback function for deactivate transition
  CLUSTER_NODE_PUBLIC
  virtual void on_deactivated() = 0;

  /// Callback function for standby transition
  CLUSTER_NODE_PUBLIC
  virtual void on_standby() = 0;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_CLUSTER_NODE_INTERFACE_HPP_
