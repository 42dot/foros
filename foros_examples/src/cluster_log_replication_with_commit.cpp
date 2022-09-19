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

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node.hpp"
#include "akit/failover/foros/cluster_node_options.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

int main(int argc, char **argv) {
  try {
    const std::string kClusterName = "test_cluster_log";

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

    unsigned char ch = 'a';

    rclcpp::init(argc, argv);

    auto options = akit::failover::foros::ClusterNodeOptions();
    options.election_timeout_max(2000);
    options.election_timeout_min(1500);

    auto node = akit::failover::foros::ClusterNode::make_shared(
        kClusterName, id, cluster_node_ids, options);

    node->register_on_activated([&]() { RCLCPP_INFO(logger, "activated"); });
    node->register_on_deactivated(
        [&]() { RCLCPP_INFO(logger, "deactivated"); });
    node->register_on_standby([&]() { RCLCPP_INFO(logger, "standby"); });
    node->register_on_committed(
        [&](int64_t id, akit::failover::foros::Command::SharedPtr command) {
          RCLCPP_INFO(logger, "command commited : %ld, %c", id,
                      command->data()[0]);
        });
    node->register_on_reverted([&](int64_t id) {
      RCLCPP_INFO(logger, "command reverted until : %ld", id);
    });

    auto timer_ =
        rclcpp::create_timer(node, rclcpp::Clock::make_shared(), 2s, [&]() {
          node->commit_command(
              akit::failover::foros::Command::make_shared(
                  std::initializer_list<uint8_t>{ch}),
              [&](akit::failover::foros::CommandCommitResponseSharedFuture
                      response_future) {
                auto response = response_future.get();
                if (response->result() == true) {
                  RCLCPP_INFO(logger, "commit completed: %lu %c",
                              response->id(), ch);
                  ch++;
                } else {
                  RCLCPP_ERROR(logger, "commit failed: %lu %c", response->id(),
                               ch);
                }
              });
        });

    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
  } catch (...) {
    // Unknown exceptions
  }

  return 0;
}
