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

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "akit/failsafe/fsros/cluster_node.hpp"
#include "akit/failsafe/fsros/cluster_node_options.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char **argv) {
  uint32_t id = 1;

  if (argc >= 2) {
    id = std::stoul(argv[1]);
    if (id > 4 || id == 0) {
      std::cerr << "please use id out of 1, 2, 3, 4" << std::endl;
    }
  }

  rclcpp::init(argc, argv);

  auto options = akit::failsafe::fsros::ClusterNodeOptions();
  auto node = std::make_shared<akit::failsafe::fsros::ClusterNode>(
      id, "test_cluster", std::initializer_list<uint32_t>{1, 2, 3, 4}, options);

  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();

  return 0;
}
