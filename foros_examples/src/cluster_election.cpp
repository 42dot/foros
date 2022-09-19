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

int main(int argc, char **argv) {
  try {
    const std::string kClusterName = "test_cluster_election";

    rclcpp::Logger logger = rclcpp::get_logger(argv[0]);
    logger.set_level(rclcpp::Logger::Level::Info);

    if (argc < 3) {
      RCLCPP_ERROR(logger, "Usage : %s {node ID} {size of cluster}", argv[0]);
      return -1;
    }

    uint32_t id = std::stoul(argv[1]);
    uint32_t cluster_size = std::stoul(argv[2]);
    std::vector<uint32_t> cluster_node_ids;

    if (id >= cluster_size) {
      RCLCPP_ERROR(logger, "ID must be less than cluster size");
      return -1;
    }

    for (uint32_t i = 0; i < cluster_size; i++) {
      cluster_node_ids.push_back(i);
    }

    rclcpp::init(argc, argv);

    auto node = std::make_shared<akit::failover::foros::ClusterNode>(
        kClusterName, id, cluster_node_ids);

    node->register_on_activated([&]() { RCLCPP_INFO(logger, "activated"); });
    node->register_on_deactivated(
        [&]() { RCLCPP_INFO(logger, "deactivated"); });
    node->register_on_standby([&]() { RCLCPP_INFO(logger, "standby"); });

    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
  } catch (...) {
    // Unknown exceptions
  }

  return 0;
}
