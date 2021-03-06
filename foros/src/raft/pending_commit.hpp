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

#ifndef AKIT_FAILOVER_FOROS_RAFT_PENDING_COMMIT_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_PENDING_COMMIT_HPP_

#include <map>

#include "raft/commit_info.hpp"
#include "raft/log_entry.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class PendingCommit {
 public:
  PendingCommit(LogEntry::SharedPtr log,
                CommandCommitResponseSharedPromise promise,
                CommandCommitResponseSharedFuture future,
                CommandCommitResponseCallback callback)
      : log_(log), promise_(promise), future_(future), callback_(callback) {}

  const LogEntry::SharedPtr log_;
  CommandCommitResponseSharedPromise promise_;
  CommandCommitResponseSharedFuture future_;
  CommandCommitResponseCallback callback_;
  std::map<uint32_t, bool> result_map_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_PENDING_COMMIT_HPP_
