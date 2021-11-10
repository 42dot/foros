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

#ifndef AKIT_FAILOVER_FOROS_COMMIT_HPP_
#define AKIT_FAILOVER_FOROS_COMMIT_HPP_

#include <rclcpp/macros.hpp>

#include <functional>
#include <future>
#include <memory>
#include <utility>
#include <vector>

namespace akit {
namespace failover {
namespace foros {

class CommitData {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(CommitData)

  uint64_t commit_id_;
  std::vector<uint8_t> data_;
};

class CommitResponse {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(CommitResponse)

  uint64_t commit_id_;
  bool result;
};

using CommitResponsePromise = std::promise<CommitResponse::SharedPtr>;
using CommitResponseSharedPromise = std::shared_ptr<CommitResponsePromise>;
using CommitResponseSharedFuture =
    std::shared_future<CommitResponse::SharedPtr>;
using CommitResponseCallback = std::function<void(CommitResponseSharedFuture)>;

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_COMMIT_HPP_
