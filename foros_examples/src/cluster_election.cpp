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

#include "akit/failover/foros/cluster_node.hpp"
#include "akit/failover/foros/cluster_node_options.hpp"
#include "rclcpp/rclcpp.hpp"

class MyClusterNode : public akit::failover::foros::ClusterNode {
 public:
  MyClusterNode(const std::string &cluster_name, const uint32_t node_id,
                const std::vector<uint32_t> &cluster_node_ids)
      : akit::failover::foros::ClusterNode(cluster_name, node_id,
                                           cluster_node_ids) {}

  void on_activated() override { std::cout << "activated" << std::endl; }

  void on_deactivated() override { std::cout << "deactivated" << std::endl; }

  void on_standby() override { std::cout << "standby" << std::endl; }
};

int main(int argc, char **argv) {
  uint32_t id = 1;
  const std::string cluster_name = "test_cluster";
  const std::vector<uint32_t> cluster_node_ids = {1, 2, 3, 4};

  if (argc >= 2) {
    id = std::stoul(argv[1]);
    if (id > 4 || id == 0) {
      std::cerr << "please use id out of 1, 2, 3, 4" << std::endl;
    }
  }

  rclcpp::init(argc, argv);

  auto node =
      std::make_shared<MyClusterNode>(cluster_name, id, cluster_node_ids);

  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();

  return 0;
}
