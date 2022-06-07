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

#ifndef AKIT_FAILOVER_FOROS_RAFT_CONTEXT_STORE_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_CONTEXT_STORE_HPP_

#include <leveldb/db.h>
#include <rclcpp/logger.hpp>

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/command.hpp"
#include "raft/log_entry.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class ContextStore final {
 public:
  explicit ContextStore(const std::string &path, rclcpp::Logger &logger);
  ~ContextStore();

  bool current_term(const uint64_t term);
  uint64_t current_term() const;

  bool voted_for(const uint32_t id);
  uint32_t voted_for() const;

  bool voted(const bool voted);
  bool voted() const;

  uint32_t vote_received();
  bool increase_vote_received();
  bool reset_vote_received();

  const LogEntry::SharedPtr log(const uint64_t id);
  const LogEntry::SharedPtr log();
  bool push_log(LogEntry::SharedPtr log);
  bool revert_log(const uint64_t id);
  uint64_t logs_size() const;

 private:
  void init_current_term();
  void init_voted_for();
  void init_voted();
  void init_logs();
  bool store_logs_size(const uint64_t size);
  uint64_t load_logs_size();
  LogEntry::SharedPtr load_log(const uint64_t id);
  bool store_log_term(const uint64_t id, const uint64_t term);
  bool store_log_data(const uint64_t id, std::vector<uint8_t> data);
  std::string get_log_data_key(const uint64_t id);
  std::string get_log_term_key(const uint64_t id);

  const char *kCurrentTermKey = "current_term";
  const char *kVotedForKey = "voted_for";
  const char *kVotedKey = "voted";
  const char *kLogKeyPrefix = "log/";
  const char *kLogDataKeySuffix = "/data";
  const char *kLogTermKeySuffix = "/term";
  const char *kLogSizeKey = "log_size";

  leveldb::DB *db_;

  uint64_t current_term_;
  uint32_t voted_for_;
  bool voted_;
  uint32_t vote_received_;

  std::vector<LogEntry::SharedPtr> logs_;

  rclcpp::Logger logger_;

  mutable std::mutex store_mutex_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_CONTEXT_STORE_HPP_
