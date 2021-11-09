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
#include <std_srvs/srv/trigger.hpp>

#include <chrono>
#include <memory>
#include <string>

using namespace std::chrono_literals;

static void on_response(
    rclcpp::Client<std_srvs::srv::Trigger>::SharedFutureWithRequest future) {
  auto ret = future.get();
  auto response = ret.second;

  std::cout << "response received from " << response->message << std::endl;
}

int main(int argc, char **argv) {
  const std::string kNodeName = "test_cluster_client";
  const std::string kServiceName = "test_cluster_get_leader_name";

  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared(kNodeName);
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr client =
      node->create_client<std_srvs::srv::Trigger>(kServiceName);

  auto timer_ =
      rclcpp::create_timer(node, rclcpp::Clock::make_shared(), 1s, [&]() {
        auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
        auto ret = client->async_send_request(request, on_response);
      });

  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();

  return 0;
}
