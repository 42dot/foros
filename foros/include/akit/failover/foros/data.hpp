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

#ifndef AKIT_FAILOVER_FOROS_DATA_HPP_
#define AKIT_FAILOVER_FOROS_DATA_HPP_

#include <rclcpp/macros.hpp>

#include <functional>
#include <future>
#include <memory>
#include <utility>
#include <vector>

namespace akit {
namespace failover {
namespace foros {

/// Data to commit into the cluster.
class Data {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(Data)

  /// Create a new Data.
  Data();

  /// Create a new data with the specified ID, data
  /**
   * \param[in] id ID of the data.
   * \param[in] data Data in byte vector.
   */
  Data(uint64_t id, std::vector<uint8_t> data);

  /// Create a new data with the specified ID, data
  /**
   * \param[in] id ID of the data.
   * \param[in] sub_id Sub ID of the data.
   * \param[in] data Data in byte vector.
   */
  Data(uint64_t id, uint64_t sub_id, std::vector<uint8_t> data);

  /// Get ID of the data.
  const uint64_t &id();

  /// Get sub ID of the data.
  const uint64_t &sub_id();

  /// Get raw data.
  const std::vector<uint8_t> &data();

 private:
  uint64_t id_;
  uint64_t sub_id_;
  std::vector<uint8_t> data_;
};

/// Response of request to commit data into the cluster.
class DataCommitResponse {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(DataCommitResponse)

  // Commit index of the requested data.
  uint64_t commit_index_;
  // Result of the request.
  bool result_;
};

using DataCommitResponsePromise = std::promise<DataCommitResponse::SharedPtr>;
using DataCommitResponseSharedPromise =
    std::shared_ptr<DataCommitResponsePromise>;
using DataCommitResponseSharedFuture =
    std::shared_future<DataCommitResponse::SharedPtr>;
using DataCommitResponseCallback =
    std::function<void(DataCommitResponseSharedFuture)>;

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_DATA_HPP_
