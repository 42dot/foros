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

#ifndef AKIT_FAILOVER_FOROS_RAFT_LOG_ENTRY_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_LOG_ENTRY_HPP_

#include <rclcpp/macros.hpp>

#include "akit/failover/foros/command.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class LogEntry {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(LogEntry)

  LogEntry(uint64_t id, uint64_t term, Command::SharedPtr command)
      : id_(id), term_(term), command_(command) {}

  const uint64_t id_;
  const uint64_t term_;
  const Command::SharedPtr command_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_LOG_ENTRY_HPP_
