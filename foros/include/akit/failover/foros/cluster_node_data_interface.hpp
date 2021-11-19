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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_DATA_INTERFACE_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_DATA_INTERFACE_HPP_

#include <vector>

#include "akit/failover/foros/common.hpp"
#include "akit/failover/foros/data.hpp"

namespace akit {
namespace failover {
namespace foros {

/// Data interface of a clustered node.
/**
 * This interface consists of pure virtual methods related with data
 * replication.
 */
class ClusterNodeDataInterface {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(ClusterNodeDataInterface)

  /// Callback function to handle the request to commit data.
  /**
   * \param[in] data Data to commit.
   * \return true if data is committed, false if not.
   */
  virtual bool on_data_commit_requested(Data::SharedPtr data) = 0;

  /// Callback function to handle the request to get data.
  /**
   * \param[in] id ID of the data.
   * \return Data of given ID, null if data does not exist.
   */
  virtual Data::SharedPtr on_data_get_requested(uint64_t id) = 0;

  /// Callback function to handle the request to get data.
  /**
   * \return The last data, null if data does not exist.
   */
  virtual Data::SharedPtr on_data_get_requested() = 0;

  /// Callback function to handle the request to rollback data.
  /**
   * \param[in] id ID of the data to rollback.
   */
  virtual void on_data_rollback_requested(uint64_t id) = 0;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_DATA_INTERFACE_HPP_
