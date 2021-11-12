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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_INTERFACE_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_INTERFACE_HPP_

#include <vector>

#include "akit/failover/foros/common.hpp"

namespace akit {
namespace failover {
namespace foros {

enum class ClusterNodeState {
  kActive,
  kInactive,
  kStandby,
  kUnknown,
};

/// Lifecycle interface of a clustered node.
/**
 * This interface consists of pure virtual methods related with node's
 * lifecycle.
 */
class ClusterNodeInterface {
 public:
  /// Callback function for activate transition.
  virtual void on_activated() = 0;

  /// Callback function for deactivate transition.
  virtual void on_deactivated() = 0;

  /// Callback function for standby transition.
  virtual void on_standby() = 0;

  /// Check whether the node is activated or not.
  /**
   * \return true if the node is activated, false if not.
   */
  virtual bool is_activated() = 0;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_INTERFACE_HPP_
