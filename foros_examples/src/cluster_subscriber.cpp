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

static void on_topic_received(const std_msgs::msg::String::SharedPtr msg) {
  std::cout << "topic received from " << msg->data << std::endl;
}

int main(int argc, char **argv) {
  const std::string kNodeName = "test_cluster_subscriber";
  const std::string kTopicName = "test_cluster_echo";

  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared(kNodeName);

  auto subscription = node->create_subscription<std_msgs::msg::String>(
      kTopicName, 10, on_topic_received);

  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();

  return 0;
}
