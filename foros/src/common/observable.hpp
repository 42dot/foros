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

#ifndef AKIT_FAILOVER_FOROS_COMMON_OBSERVABLE_HPP_
#define AKIT_FAILOVER_FOROS_COMMON_OBSERVABLE_HPP_

#include <list>
#include <mutex>

#include "common/observer.hpp"

namespace akit {
namespace failover {
namespace foros {

template <typename T>
class Observable {
 public:
  void subscribe(Observer<T> *observer) {
    std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
    observers_.push_back(observer);
  }

  void unsubscribe(Observer<T> *observer) {
    std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
    observers_.remove(observer);
  }

  void notify(const T &data) {
    std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
    for (auto observer : observers_) {
      observer->handle(data);
    }
  }

 private:
  std::list<Observer<T> *> observers_;
  std::recursive_mutex callback_mutex_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_COMMON_OBSERVABLE_HPP_
