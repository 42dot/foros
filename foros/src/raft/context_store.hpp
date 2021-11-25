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

#include <memory>
#include <string>

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class ContextStore final {
 public:
  explicit ContextStore(std::string &path, rclcpp::Logger &logger);
  ~ContextStore();

  bool current_term(uint64_t term);
  uint64_t current_term() const;

  bool voted_for(uint32_t id);
  uint32_t voted_for() const;

  bool voted(bool voted);
  bool voted() const;

 private:
  void init_current_term();
  void init_voted_for();
  void init_voted();

  const char *kCurrentTermKey = "current_term";
  const char *kVotedForKey = "voted_for";
  const char *kVotedKey = "voted";

  leveldb::DB *db_;
  rclcpp::Logger logger_;

  uint64_t current_term_;
  uint32_t voted_for_;
  bool voted_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_CONTEXT_STORE_HPP_
