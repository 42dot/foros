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

#include <gtest/gtest.h>
#include <std_msgs/msg/empty.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "akit/failsafe/fsros/cluster_node.hpp"
#include "akit/failsafe/fsros/cluster_node_publisher.hpp"

namespace fsros = akit::failsafe::fsros;

class MyClusterNode : public fsros::ClusterNode {
 public:
  explicit MyClusterNode(const std::string &cluster_name,
                         const std::string &node_name,
                         const std::vector<std::string> &cluster_node_names)
      : akit::failsafe::fsros::ClusterNode(cluster_name, node_name,
                                           cluster_node_names) {
    rclcpp::PublisherOptionsWithAllocator<std::allocator<void>> options;

    publisher_ = create_publisher<std_msgs::msg::Empty>("test_pub", 10);
  }

  void on_activated() override {}

  void on_deactivated() override {}

  void on_standby() override {}

  auto publisher() { return publisher_; }

 private:
  fsros::ClusterNodePublisher<std_msgs::msg::Empty>::SharedPtr publisher_;
};

class TestPublisher : public ::testing::Test {
 public:
  void SetUp() {
    rclcpp::init(0, nullptr);
    node_ = std::make_shared<MyClusterNode>(
        "node1", "test_cluster",
        std::initializer_list<std::string>{"node1", "node2", "node3", "node4"});
  }

  void TearDown() {
    node_.reset();
    rclcpp::shutdown();
  }

 protected:
  std::shared_ptr<MyClusterNode> node_;
};

TEST_F(TestPublisher, TestPublisherLifecycle) {
  node_->publisher()->on_deactivated();
  EXPECT_TRUE(node_->publisher()->get_current_state() ==
              fsros::ClusterNodeState::kInactive);
  {
    auto msg_ptr = std::make_unique<std_msgs::msg::Empty>();
    EXPECT_NO_THROW(node_->publisher()->publish(*msg_ptr));
  }
  {
    auto msg_ptr = std::make_unique<std_msgs::msg::Empty>();
    EXPECT_NO_THROW(node_->publisher()->publish(std::move(msg_ptr)));
  }

  node_->publisher()->on_activated();
  EXPECT_TRUE(node_->publisher()->get_current_state() ==
              fsros::ClusterNodeState::kActive);
  {
    auto msg_ptr = std::make_unique<std_msgs::msg::Empty>();
    EXPECT_NO_THROW(node_->publisher()->publish(*msg_ptr));
  }
  {
    auto msg_ptr = std::make_unique<std_msgs::msg::Empty>();
    EXPECT_NO_THROW(node_->publisher()->publish(std::move(msg_ptr)));
  }
}
