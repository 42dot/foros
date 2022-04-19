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

#ifndef AKIT_FAILOVER_FOROS_INSPECTOR_CLUSTER_INFO_HPP_
#define AKIT_FAILOVER_FOROS_INSPECTOR_CLUSTER_INFO_HPP_

#include <rclcpp/rclcpp.hpp>

#include <map>
#include <memory>
#include <string>

#include "node_info.hpp"

namespace akit {
namespace failover {
namespace foros_inspector {

class ClusterInfo {
 public:
  explicit ClusterInfo(std::string name)
      : name_(name),
        size_(0),
        term_(0),
        leader_(0),
        leader_exist_(false),
        size_mismatched_(false),
        last_updated_(0, 0, RCL_ROS_TIME) {}

  std::string name_;
  uint32_t size_;
  uint64_t term_;
  uint32_t leader_;
  bool leader_exist_;
  bool size_mismatched_;
  std::map<uint32_t, std::shared_ptr<NodeInfo>> nodes_;
  rclcpp::Time last_updated_;
};

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_INSPECTOR_CLUSTER_INFO_HPP_
