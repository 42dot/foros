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

#ifndef AKIT_FAILOVER_FOROS_COMMAND_HPP_
#define AKIT_FAILOVER_FOROS_COMMAND_HPP_

#include <rclcpp/macros.hpp>

#include <functional>
#include <future>
#include <memory>
#include <utility>
#include <vector>

namespace akit {
namespace failover {
namespace foros {

/// Command.
class Command {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(Command)

  /// Create a command.
  /**
   * \param[in] data Data in byte vector.
   */
  explicit Command(std::vector<uint8_t> data);

  /// Create a command
  /**
   * \param[in] data Data in byte pointer.
   * \param[in] size Size of the data.
   */
  explicit Command(const char *data, uint64_t size);

  /// Get the data.
  /**
   * \return The data in byte vector.
   */
  const std::vector<uint8_t> &data() const;

 private:
  std::vector<uint8_t> data_;
};

/// A response of a request to commit a command to the cluster.
class CommandCommitResponse {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(CommandCommitResponse)

  /// Create a response of a request to commit.
  /**
   * \param[in] id A command ID.
   * \param[in] command A command.
   * \param[in] result true if the commit is successful, otherwise false.
   */
  explicit CommandCommitResponse(uint64_t id, Command::SharedPtr command,
                                 bool result);

  /// Get the ID.
  /**
   * \return id the ID.
   */
  uint64_t id() const;

  /// Get the command.
  /**
   * \return The command.
   */
  const Command::SharedPtr command() const;

  /// Get the result.
  /**
   * \return true if the commit is successful, otherwise false.
   */
  bool result() const;

 private:
  const uint64_t id_;
  const Command::SharedPtr command_;
  const bool result_;
};

using CommandCommitResponsePromise =
    std::promise<CommandCommitResponse::SharedPtr>;
using CommandCommitResponseSharedPromise =
    std::shared_ptr<CommandCommitResponsePromise>;
using CommandCommitResponseSharedFuture =
    std::shared_future<CommandCommitResponse::SharedPtr>;
using CommandCommitResponseCallback =
    std::function<void(CommandCommitResponseSharedFuture)>;

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_COMMAND_HPP_
