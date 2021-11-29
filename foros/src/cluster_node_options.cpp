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

#include "akit/failover/foros/cluster_node_options.hpp"

#include <filesystem>
#include <string>

#include "akit/failover/foros/common.hpp"

namespace akit {
namespace failover {
namespace foros {

ClusterNodeOptions::ClusterNodeOptions(rcl_allocator_t allocator)
    : NodeOptions(allocator),
      election_timeout_min_(150),
      election_timeout_max_(300),
      temp_directory_(std::filesystem::temp_directory_path()) {}

unsigned int ClusterNodeOptions::election_timeout_min() const {
  return election_timeout_min_;
}

ClusterNodeOptions &ClusterNodeOptions::election_timeout_min(unsigned int min) {
  election_timeout_min_ = min;
  return *this;
}

unsigned int ClusterNodeOptions::election_timeout_max() const {
  return election_timeout_max_;
}

ClusterNodeOptions &ClusterNodeOptions::election_timeout_max(unsigned int max) {
  election_timeout_max_ = max;
  return *this;
}

std::string ClusterNodeOptions::temp_directory() const {
  return temp_directory_;
}

ClusterNodeOptions &ClusterNodeOptions::temp_directory(std::string &directory) {
  temp_directory_ = directory;
  return *this;
}

}  // namespace foros
}  // namespace failover
}  // namespace akit
