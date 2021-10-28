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
#include <string>
#include <vector>

#include "akit/failsafe/fsros/cluster_node.hpp"

class TestNodeCluster : public ::testing::Test {
 protected:
  static void SetUpTestCase() { rclcpp::init(0, nullptr); }

  static void TearDownTestCase() { rclcpp::shutdown(); }
};

/*
   Testing node constructor
 */
TEST_F(TestNodeCluster, TestConstructor) {
  {
    auto cluster_node = std::make_shared<akit::failsafe::fsros::ClusterNode>(
        1, "test_cluster", std::initializer_list<uint32_t>{1, 2, 3, 4});
  }

  {
    ASSERT_THROW(
        {
          auto cluster_node =
              std::make_shared<akit::failsafe::fsros::ClusterNode>(
                  1, "invalid_cluster?",
                  std::initializer_list<uint32_t>{1, 2, 3, 4});

          (void)cluster_node;
        },
        rclcpp::exceptions::InvalidNamespaceError);
  }
}
