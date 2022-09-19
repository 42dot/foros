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

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <string>

using namespace std::chrono_literals;

int main(int argc, char **argv) {
  try {
    const std::string kNodeName = "test_cluster_subscriber";
    const std::string kTopicName = "test_cluster_echo";

    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared(kNodeName);

    rclcpp::Logger logger = rclcpp::get_logger(argv[0]);
    logger.set_level(rclcpp::Logger::Level::Info);

    auto subscription = node->create_subscription<std_msgs::msg::String>(
        kTopicName, 10, [&](const std_msgs::msg::String::SharedPtr msg) {
          RCLCPP_INFO(logger, "topic received from %s", msg->data.c_str());
        });

    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
  } catch (...) {
    // Unknown exceptions
  }

  return 0;
}
