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

#ifndef AKIT_FAILOVER_FOROS_COMMON_VOID_CALLBACK_HPP_
#define AKIT_FAILOVER_FOROS_COMMON_VOID_CALLBACK_HPP_

#include <functional>

namespace akit {
namespace failover {
namespace foros {

class VoidCallback {
 public:
  explicit VoidCallback(std::function<void()> callback) : callback_(callback) {}

  void call() {
    if (callback_ != nullptr) {
      callback_();
    }
  }

 private:
  std::function<void()> callback_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_COMMON_VOID_CALLBACK_HPP_
