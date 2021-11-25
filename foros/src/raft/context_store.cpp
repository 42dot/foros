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

#include "raft/context_store.hpp"

#include <leveldb/db.h>
#include <rclcpp/logging.hpp>

#include <memory>
#include <string>

namespace akit {
namespace failover {
namespace foros {
namespace raft {

ContextStore::ContextStore(std::string &path, rclcpp::Logger &logger)
    : logger_(logger.get_child("raft")) {
  leveldb::Options options;
  options.create_if_missing = true;
  auto status = leveldb::DB::Open(options, path, &db_);

  if (status.ok() == false || db_ == nullptr) {
    RCLCPP_ERROR(logger_, "Context store DB open failed: %s",
                 status.ToString().c_str());
    return;
  }

  init_current_term();
  init_voted_for();
  init_voted();
}

ContextStore::~ContextStore() {
  if (db_ != nullptr) {
    delete db_;
  }
}

bool ContextStore::current_term(uint64_t term) {
  current_term_ = term;
  leveldb::Slice value((char *)&term, sizeof(uint64_t));
  auto status = db_->Put(leveldb::WriteOptions(), kCurrentTermKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "current_term set failed: %s",
                 status.ToString().c_str());
    return false;
  }
  return true;
}

uint64_t ContextStore::current_term() const { return current_term_; }

void ContextStore::init_current_term() {
  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kCurrentTermKey, &value);

  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "current_term get failed: %s",
                 status.ToString().c_str());
    current_term_ = 0;
    return;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(uint64_t)) {
    RCLCPP_ERROR(logger_, "current_term value size is invalid");
    current_term_ = 0;
    return;
  }

  current_term_ = *((uint64_t *)slice.data());
}

bool ContextStore::voted_for(uint32_t id) {
  voted_for_ = id;
  leveldb::Slice value((char *)&id, sizeof(uint32_t));
  auto status = db_->Put(leveldb::WriteOptions(), kVotedForKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "voted_for set failed: %s",
                 status.ToString().c_str());
    return false;
  }
  return true;
}

uint32_t ContextStore::voted_for() const { return voted_for_; }

void ContextStore::init_voted_for() {
  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kVotedForKey, &value);

  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "voted_for get failed: %s",
                 status.ToString().c_str());
    voted_for_ = 0;
    return;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(uint32_t)) {
    RCLCPP_ERROR(logger_, "voted_for value size is invalid");
    voted_for_ = 0;
    return;
  }

  voted_for_ = *((uint32_t *)slice.data());
}

bool ContextStore::voted(bool voted) {
  voted_ = voted;
  leveldb::Slice value((char *)&voted, sizeof(bool));
  auto status = db_->Put(leveldb::WriteOptions(), kVotedKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "voted set failed: %s", status.ToString().c_str());
    return false;
  }
  return true;
}

bool ContextStore::voted() const { return voted_; }

void ContextStore::init_voted() {
  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kVotedKey, &value);

  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "voted get failed: %s", status.ToString().c_str());
    voted_ = false;
    return;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(bool)) {
    RCLCPP_ERROR(logger_, "voted value size is invalid");
    voted_ = false;
    return;
  }

  voted_ = *((bool *)slice.data());
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
