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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node.hpp"

using namespace std::chrono_literals;

class TestClusterNode : public ::testing::Test {
 protected:
  static void SetUpTestCase() { rclcpp::init(0, nullptr); }

  static void TearDownTestCase() { rclcpp::shutdown(); }

  const char *kClusterName = "test_cluster";
  const char *kNamespace = "/ns_cluster";
  const char *kInvalidNodeName = "invalid_cluster?";
  const std::string kStorePath = "/tmp/foros_test_cluster0";
  const uint32_t kNodeId = 0;
  const std::vector<uint32_t> kClusterIds = std::initializer_list<uint32_t>{0};
  const std::vector<uint32_t> kClusterIds2 =
      std::initializer_list<uint32_t>{0, 1};
  const uint8_t kTestData = 'a';
  rclcpp::Logger logger_ = rclcpp::get_logger("test_raft");
};

TEST_F(TestClusterNode, TestConstructor) {
  {
    auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
        kClusterName, kNodeId, kClusterIds2);
  }

  {
    ASSERT_THROW(
        {
          auto cluster_node =
              std::make_shared<akit::failover::foros::ClusterNode>(
                  kInvalidNodeName, kNodeId, kClusterIds2);

          (void)cluster_node;
        },
        rclcpp::exceptions::InvalidNodeNameError);
  }
}

TEST_F(TestClusterNode, TestClusterNodeOptions) {
  const unsigned int kTimeoutMax = 2000;
  const unsigned int kTimeoutMin = 1000;
  auto options = akit::failover::foros::ClusterNodeOptions();

  options.election_timeout_max(kTimeoutMax);
  EXPECT_EQ(kTimeoutMax, options.election_timeout_max());
  options.election_timeout_min(kTimeoutMin);
  EXPECT_EQ(kTimeoutMin, options.election_timeout_min());

  std::string kTempDirectory = "/tmp";
  options.temp_directory(kTempDirectory);
  EXPECT_EQ(kTempDirectory, options.temp_directory());
}

TEST_F(TestClusterNode, TestGetNodeInfo) {
  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds2, kNamespace);
  EXPECT_EQ(std::string(cluster_node->get_name()),
            std::string(kClusterName + std::to_string(kNodeId)));
  EXPECT_EQ(std::string(cluster_node->get_namespace()),
            std::string(kNamespace));
  EXPECT_EQ(std::string(cluster_node->get_fully_qualified_name()),
            std::string(cluster_node->get_namespace()) + std::string("/") +
                std::string(cluster_node->get_name()));
}

TEST_F(TestClusterNode, TestLifecycleCallbacks) {
  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds, kNamespace);

  testing::MockFunction<void()> on_activated_callback;
  testing::MockFunction<void()> on_standby_callback;
  testing::MockFunction<void()> on_deactivated_callback;

  EXPECT_CALL(on_activated_callback, Call()).WillOnce(testing::Return());
  EXPECT_CALL(on_standby_callback, Call()).Times(0);
  EXPECT_CALL(on_deactivated_callback, Call()).Times(0);

  cluster_node->register_on_activated(on_activated_callback.AsStdFunction());
  cluster_node->register_on_standby(on_standby_callback.AsStdFunction());
  cluster_node->register_on_deactivated(
      on_deactivated_callback.AsStdFunction());
  EXPECT_EQ(cluster_node->is_activated(), false);

  bool running = true;
  rclcpp::TimerBase::SharedPtr timer =
      rclcpp::create_timer(cluster_node, rclcpp::Clock::make_shared(), 1s,
                           [&]() { running = false; });

  rclcpp::WallRate loop_rate(100ms);
  while (running && rclcpp::ok()) {
    rclcpp::spin_some(cluster_node->get_node_base_interface());
    loop_rate.sleep();
  }

  EXPECT_EQ(cluster_node->is_activated(), true);
}

TEST_F(TestClusterNode, TestInterfaceGetters) {
  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds, kNamespace);

  EXPECT_NE(cluster_node->get_clock(), nullptr);

  EXPECT_NE(cluster_node->get_node_base_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_clock_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_graph_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_logging_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_time_source_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_timers_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_topics_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_services_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_parameters_interface(), nullptr);
  EXPECT_NE(cluster_node->get_node_waitables_interface(), nullptr);
}

TEST_F(TestClusterNode, TestCommandCommit) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error &err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds, kNamespace);

  EXPECT_EQ(cluster_node->get_commands_size(), (uint64_t)0);

  testing::MockFunction<void(
      akit::failover::foros::CommandCommitResponseSharedFuture)>
      on_commit_response;
  testing::MockFunction<void(const uint64_t)> on_reverted_callback;
  testing::MockFunction<void(const uint64_t,
                             akit::failover::foros::Command::SharedPtr)>
      on_committed_callback;

  EXPECT_CALL(on_commit_response, Call(testing::_)).WillOnce(testing::Return());
  EXPECT_CALL(on_committed_callback, Call((uint64_t)0, testing::_))
      .WillOnce(testing::Return());
  EXPECT_CALL(on_reverted_callback, Call(testing::_)).Times(0);

  cluster_node->register_on_committed(on_committed_callback.AsStdFunction());
  cluster_node->register_on_reverted(on_reverted_callback.AsStdFunction());

  rclcpp::WallRate loop_rate(100ms);
  while (!cluster_node->is_activated() && rclcpp::ok()) {
    rclcpp::spin_some(cluster_node->get_node_base_interface());
    loop_rate.sleep();
  }

  auto future = cluster_node->commit_command(
      akit::failover::foros::Command::make_shared(
          std::initializer_list<uint8_t>{kTestData}),
      on_commit_response.AsStdFunction());
  rclcpp::spin_until_future_complete(cluster_node->get_node_base_interface(),
                                     future, 1s);

  auto response = future.get();
  EXPECT_EQ(response->id(), (uint64_t)0);
  EXPECT_EQ(response->result(), true);
  EXPECT_EQ(response->command()->data()[0], kTestData);

  EXPECT_NE(cluster_node->get_command(0), nullptr);
  EXPECT_EQ(cluster_node->get_commands_size(), (uint64_t)1);
}

TEST_F(TestClusterNode, TestParameter) {
  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds, kNamespace);

  const std::string kParamName = "param";
  const int kParamValue = 1;

  cluster_node->declare_parameter(kParamName, kParamValue);
  EXPECT_EQ(cluster_node->has_parameter(kParamName), true);

  auto parameter = cluster_node->get_parameter(kParamName);
  EXPECT_EQ(parameter.get_name(), kParamName);
}

TEST_F(TestClusterNode, TestParametersWithoutDeclaration) {
  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds, kNamespace);

  const std::string kParamNamePrefix = "param";
  const unsigned int kParameterSize = 10;

  std::vector<rclcpp::Parameter> parameters;

  for (unsigned int i = 0; i < kParameterSize; i++) {
    parameters.emplace_back(std::string(kParamNamePrefix + std::to_string(i)));
  }

  ASSERT_THROW({ auto result = cluster_node->set_parameters(parameters); },
               rclcpp::exceptions::ParameterNotDeclaredException);
}

TEST_F(TestClusterNode, TestClusterNodeLogger) {
  auto cluster_node = std::make_shared<akit::failover::foros::ClusterNode>(
      kClusterName, kNodeId, kClusterIds, kNamespace);

  const std::string kChildName = "child";
  const std::string kNameSuffix = ".child";

  auto logger = cluster_node->get_logger();
  EXPECT_EQ(std::string(logger.get_name() + kNameSuffix),
            std::string(logger.get_child(kChildName).get_name()));
}
