/*
 * Copyright (c) 2022 42dot All rights reserved.
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

#include <rclcpp/rclcpp.hpp>

#include <memory>

#include "inspector.hpp"

int main(int argc, char **argv) {
  try {
    rclcpp::init(argc, argv);

    auto inspector =
        std::make_shared<akit::failover::foros_inspector::Inspector>();

    rclcpp::spin(inspector);
    rclcpp::shutdown();
  } catch (...) {
    // Unknown exceptions
  }

  return 0;
}
