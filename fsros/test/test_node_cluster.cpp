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
#include <rclcpp/rclcpp.hpp>

#include <memory>

#include "akit/failsafe/fsros/node_cluster.hpp"

class TestNodeCluster : public ::testing::Test {
 protected:
  static void SetUpTestCase() { rclcpp::init(0, nullptr); }

  static void TearDownTestCase() { rclcpp::shutdown(); }
};

/*
   Testing node constor
 */
TEST_F(TestNodeCluster, TestConstructor) {
  {
    auto node_cluster =
        std::make_shared<akit::failsafe::fsros::NodeCluster>("test_node");
  }

  {
    auto node_cluster = std::make_shared<akit::failsafe::fsros::NodeCluster>(
        "test_node", "/test_ns");
  }

  {
    ASSERT_THROW(
        {
          auto node_cluster =
              std::make_shared<akit::failsafe::fsros::NodeCluster>("");
          (void)node_cluster;
        },
        rclcpp::exceptions::InvalidNodeNameError);
  }

  {
    ASSERT_THROW(
        {
          auto node_cluster =
              std::make_shared<akit::failsafe::fsros::NodeCluster>(
                  "invalid_node?");
          (void)node_cluster;
        },
        rclcpp::exceptions::InvalidNodeNameError);
  }

  {
    ASSERT_THROW(
        {
          auto node_cluster =
              std::make_shared<akit::failsafe::fsros::NodeCluster>(
                  "test_node", "/invalid_ns?");
          (void)node_cluster;
        },
        rclcpp::exceptions::InvalidNamespaceError);
  }
}
