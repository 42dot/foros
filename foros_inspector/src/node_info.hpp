/*
 * Copyright (c) 2022 42dot All rights reserved.
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

#ifndef AKIT_FAILOVER_FOROS_INSPECTOR_NODE_INFO_HPP_
#define AKIT_FAILOVER_FOROS_INSPECTOR_NODE_INFO_HPP_

#include <rclcpp/rclcpp.hpp>

namespace akit {
namespace failover {
namespace foros_inspector {

class NodeInfo {
 public:
  explicit NodeInfo(uint32_t id)
      : id_(id),
        size_(0),
        state_(0),
        term_(0),
        data_size_(0),
        voted_for_(0),
        last_updated_(0, 0, RCL_ROS_TIME) {}

  uint32_t id_;
  uint32_t size_;

  uint8_t state_;
  uint64_t term_;
  uint64_t data_size_;
  uint32_t voted_for_;

  rclcpp::Time last_updated_;
};

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_INSPECTOR_NODE_INFO_HPP_
