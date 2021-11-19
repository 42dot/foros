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

#include "akit/failover/foros/data.hpp"

#include <vector>

namespace akit {
namespace failover {
namespace foros {

Data::Data(uint64_t id, std::vector<uint8_t> data) : id_(id), data_(data) {}

Data::Data(uint64_t id, uint64_t sub_id, std::vector<uint8_t> data)
    : id_(id), sub_id_(sub_id), data_(data) {}

const uint64_t &Data::id() { return id_; }

const uint64_t &Data::sub_id() { return sub_id_; }

const std::vector<uint8_t> &Data::data() { return data_; }

}  // namespace foros
}  // namespace failover
}  // namespace akit
