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
#include <vector>

namespace akit {
namespace failover {
namespace foros {
namespace raft {

ContextStore::ContextStore(const std::string &path, rclcpp::Logger &logger)
    : current_term_(0),
      voted_for_(0),
      voted_(false),
      vote_received_(0),
      logger_(logger.get_child("raft")) {
  leveldb::Options options;
  options.create_if_missing = true;

  auto status = leveldb::DB::Open(options, path, &db_);

  if (status.ok() == false || db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db open failed: %s", status.ToString().c_str());
    return;
  }

  init_current_term();
  init_voted_for();
  init_voted();
  init_logs();
}

ContextStore::~ContextStore() {
  if (db_ != nullptr) {
    delete db_;
  }
}

bool ContextStore::current_term(const uint64_t term) {
  std::lock_guard<std::mutex> lock(store_mutex_);
  current_term_ = term;

  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return false;
  }

  leveldb::Slice value(reinterpret_cast<const char *>(&term), sizeof(uint64_t));

  auto status = db_->Put(leveldb::WriteOptions(), kCurrentTermKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "current_term set failed: %s",
                 status.ToString().c_str());
    return false;
  }
  return true;
}

uint64_t ContextStore::current_term() const {
  std::lock_guard<std::mutex> lock(store_mutex_);
  return current_term_;
}

void ContextStore::init_current_term() {
  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kCurrentTermKey, &value);

  if (status.ok() == false) {
    if (!status.NotFound) {
      RCLCPP_ERROR(logger_, "current_term get failed: %s",
                   status.ToString().c_str());
    }
    current_term_ = 0;
    return;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(uint64_t)) {
    RCLCPP_ERROR(logger_, "current_term value size is invalid");
    current_term_ = 0;
    return;
  }

  current_term_ = *(reinterpret_cast<const uint64_t *>(slice.data()));
}

bool ContextStore::voted_for(const uint32_t id) {
  std::lock_guard<std::mutex> lock(store_mutex_);
  voted_for_ = id;

  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return false;
  }

  leveldb::Slice value(reinterpret_cast<const char *>(&id), sizeof(uint32_t));
  auto status = db_->Put(leveldb::WriteOptions(), kVotedForKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "voted_for set failed: %s",
                 status.ToString().c_str());
    return false;
  }
  return true;
}

uint32_t ContextStore::voted_for() const {
  std::lock_guard<std::mutex> lock(store_mutex_);
  return voted_for_;
}

void ContextStore::init_voted_for() {
  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kVotedForKey, &value);

  if (status.ok() == false) {
    if (!status.NotFound) {
      RCLCPP_ERROR(logger_, "voted_for get failed: %s",
                   status.ToString().c_str());
    }
    voted_for_ = 0;
    return;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(uint32_t)) {
    RCLCPP_ERROR(logger_, "voted_for value size is invalid");
    voted_for_ = 0;
    return;
  }

  voted_for_ = *(reinterpret_cast<const uint32_t *>(slice.data()));
}

bool ContextStore::voted(const bool voted) {
  std::lock_guard<std::mutex> lock(store_mutex_);
  voted_ = voted;

  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return false;
  }
  leveldb::Slice value(reinterpret_cast<const char *>(&voted), sizeof(bool));
  auto status = db_->Put(leveldb::WriteOptions(), kVotedKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "voted set failed: %s", status.ToString().c_str());
    return false;
  }
  return true;
}

bool ContextStore::voted() const {
  std::lock_guard<std::mutex> lock(store_mutex_);
  return voted_;
}

void ContextStore::init_voted() {
  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kVotedKey, &value);

  if (status.ok() == false) {
    if (!status.NotFound) {
      RCLCPP_ERROR(logger_, "voted get failed: %s", status.ToString().c_str());
    }
    voted_ = false;
    return;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(bool)) {
    RCLCPP_ERROR(logger_, "voted value size is invalid");
    voted_ = false;
    return;
  }

  voted_ = *(reinterpret_cast<const bool *>(slice.data()));
}

uint32_t ContextStore::vote_received() {
  std::lock_guard<std::mutex> lock(store_mutex_);
  return vote_received_;
}

bool ContextStore::increase_vote_received() {
  std::lock_guard<std::mutex> lock(store_mutex_);
  vote_received_++;
  return true;
}

bool ContextStore::reset_vote_received() {
  std::lock_guard<std::mutex> lock(store_mutex_);
  vote_received_ = 0;
  return true;
}

const LogEntry::SharedPtr ContextStore::log(const uint64_t id) {
  std::lock_guard<std::mutex> lock(store_mutex_);
  if (logs_.size() <= id) {
    return nullptr;
  }

  return logs_[id];
}

const LogEntry::SharedPtr ContextStore::log() {
  std::lock_guard<std::mutex> lock(store_mutex_);
  if (logs_.empty() == true) {
    return nullptr;
  }

  return logs_.back();
}

uint64_t ContextStore::logs_size() const {
  std::lock_guard<std::mutex> lock(store_mutex_);
  return logs_.size();
}

uint64_t ContextStore::load_logs_size() {
  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return 0;
  }

  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), kLogSizeKey, &value);

  if (status.ok() == false) {
    if (!status.NotFound) {
      RCLCPP_ERROR(logger_, "logs size get failed: %s",
                   status.ToString().c_str());
    }
    return 0;
  }

  leveldb::Slice slice = value;
  if (slice.size() != sizeof(uint64_t)) {
    RCLCPP_ERROR(logger_, "logs size value size is invalid");
    return 0;
  }

  return *(reinterpret_cast<const uint64_t *>(slice.data()));
}

bool ContextStore::store_logs_size(const uint64_t size) {
  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return false;
  }

  leveldb::Slice value(reinterpret_cast<const char *>(&size), sizeof(uint64_t));
  auto status = db_->Put(leveldb::WriteOptions(), kLogSizeKey, value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "logs size set failed: %s",
                 status.ToString().c_str());
    return false;
  }
  return true;
}

void ContextStore::init_logs() {
  std::string value;

  logs_.clear();

  for (uint64_t i = 0; i < load_logs_size(); i++) {
    auto log = load_log(i);
    if (log == nullptr) {
      store_logs_size(i);
      break;
    }
    logs_.push_back(log);
  }
}

LogEntry::SharedPtr ContextStore::load_log(const uint64_t id) {
  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return nullptr;
  }

  std::string value;
  auto status = db_->Get(leveldb::ReadOptions(), get_log_term_key(id), &value);

  if (status.ok() == false) {
    if (!status.NotFound) {
      RCLCPP_ERROR(logger_, "log term for %lu get failed: %s", id,
                   status.ToString().c_str());
    }
    return nullptr;
  }

  leveldb::Slice slice = value;
  if (slice.size() < sizeof(uint64_t)) {
    RCLCPP_ERROR(logger_, "log term value size is invalid");
    return nullptr;
  }

  uint64_t term = *(reinterpret_cast<const uint64_t *>(slice.data()));

  status = db_->Get(leveldb::ReadOptions(), get_log_data_key(id), &value);
  slice = value;
  auto command = Command::make_shared(slice.data(), slice.size());

  return LogEntry::make_shared(id, term, command);
}

bool ContextStore::store_log_term(const uint64_t id, const uint64_t term) {
  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return false;
  }

  leveldb::Slice value(reinterpret_cast<const char *>(&term), sizeof(uint64_t));
  auto status = db_->Put(leveldb::WriteOptions(), get_log_term_key(id), value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "logs term for %lu set failed: %s", id,
                 status.ToString().c_str());
    return false;
  }

  return true;
}

bool ContextStore::store_log_data(const uint64_t id,
                                  std::vector<uint8_t> data) {
  if (db_ == nullptr) {
    RCLCPP_ERROR(logger_, "db is nullptr");
    return false;
  }

  std::string buffer(data.begin(), data.end());
  leveldb::Slice value(buffer.c_str(), data.size());
  auto status = db_->Put(leveldb::WriteOptions(), get_log_data_key(id), value);
  if (status.ok() == false) {
    RCLCPP_ERROR(logger_, "logs term for %lu set failed: %s", id,
                 status.ToString().c_str());
    return false;
  }

  return true;
}

std::string ContextStore::get_log_data_key(uint64_t id) {
  return std::string(kLogKeyPrefix + std::to_string(id) + kLogDataKeySuffix);
}

std::string ContextStore::get_log_term_key(uint64_t id) {
  return std::string(kLogKeyPrefix + std::to_string(id) + kLogTermKeySuffix);
}

bool ContextStore::push_log(LogEntry::SharedPtr log) {
  std::lock_guard<std::mutex> lock(store_mutex_);
  if (log == nullptr) {
    RCLCPP_ERROR(logger_, "log is nullptr");
    return false;
  }

  if (log->command_ == nullptr) {
    RCLCPP_ERROR(logger_, "command of log is nullptr");
    return false;
  }

  if (log->id_ != logs_.size()) {
    RCLCPP_ERROR(logger_, "log id is invalid");
    return false;
  }

  if (store_log_term(log->id_, log->term_) == false) {
    return false;
  }

  if (store_log_data(log->id_, log->command_->data()) == false) {
    return false;
  }

  if (store_logs_size(logs_.size() + 1) == false) {
    return false;
  }

  logs_.push_back(log);

  return true;
}

bool ContextStore::revert_log(const uint64_t id) {
  std::lock_guard<std::mutex> lock(store_mutex_);
  if (id >= logs_.size()) {
    RCLCPP_ERROR(logger_, "invalid id to revert: %lu", id);
    return false;
  }
  logs_.resize(id);
  store_logs_size(id);
  return true;
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
