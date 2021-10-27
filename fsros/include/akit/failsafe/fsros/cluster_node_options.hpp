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

#ifndef AKIT_FAILSAFE_FSROS_CLUSTER_NODE_OPTIONS_HPP_
#define AKIT_FAILSAFE_FSROS_CLUSTER_NODE_OPTIONS_HPP_

#include <rclcpp/node_options.hpp>

namespace akit {
namespace failsafe {
namespace fsros {

/// This contains cluster node options
struct ClusterNodeOptions {
  /// Election timeout (random beween #min and #max)
  struct {
    /// Minimum timeout (default 150ms)
    unsigned int min = 150;
    /// Maximum timeout (default 300ms)
    unsigned int max = 300;
  } election_timeout;

  rclcpp::NodeOptions node_options;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_CLUSTER_NODE_OPTIONS_HPP_
