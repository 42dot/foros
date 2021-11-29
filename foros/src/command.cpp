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

#include "akit/failover/foros/command.hpp"

#include <vector>

namespace akit {
namespace failover {
namespace foros {

Command::Command(std::vector<uint8_t> data) : data_(data) {}

Command::Command(const char *data, uint64_t size) : data_(data, data + size) {}

const std::vector<uint8_t> &Command::data() const { return data_; }

CommandCommitResponse::CommandCommitResponse(uint64_t id,
                                             Command::SharedPtr command,
                                             bool result)
    : id_(id), command_(command), result_(result) {}

uint64_t CommandCommitResponse::id() const { return id_; }

const Command::SharedPtr CommandCommitResponse::command() const {
  return command_;
}

bool CommandCommitResponse::result() const { return result_; }

}  // namespace foros
}  // namespace failover
}  // namespace akit
