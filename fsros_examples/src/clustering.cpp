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

#include <memory>
#include <string>

#include "akit/failsafe/fsros/cluster_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<akit::failsafe::fsros::ClusterNode>(
      "node1", "test_cluster",
      std::initializer_list<std::string>{"node1", "node2", "node3", "node4"});

  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
}
