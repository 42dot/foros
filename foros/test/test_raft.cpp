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
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

#include <filesystem>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node.hpp"
#include "raft/context.hpp"
#include "raft/context_store.hpp"
#include "raft/state_machine_interface.hpp"

class TestRaft : public ::testing::Test {
 protected:
  static void SetUpTestCase() { rclcpp::init(0, nullptr); }

  static void TearDownTestCase() { rclcpp::shutdown(); }

  std::string kTempPath = "/tmp";
  std::string kStorePath = "/tmp/foros_test_cluster0";
  const uint32_t kNodeId = 0;
  const char* kClusterName = "test_cluster";
  rclcpp::Logger logger_ = rclcpp::get_logger("test_raft");
  std::vector<uint32_t> kClusterIds = std::initializer_list<uint32_t>{0};
};

class TestStateMachine
    : public akit::failover::foros::raft::StateMachineInterface {
 public:
  void on_election_timedout() override {}
  void on_new_term_received() override {}
  void on_elected() override {}
  void on_broadcast_timedout() override {}
  void on_leader_discovered() override {}
  bool is_leader() override { return true; };
};

TEST_F(TestRaft, TestContextStore) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto store = akit::failover::foros::raft::ContextStore(kStorePath, logger_);

  EXPECT_EQ(store.current_term(), (uint64_t)0);
  EXPECT_EQ(store.current_term(1), true);
  EXPECT_EQ(store.current_term(), (uint64_t)1);

  EXPECT_EQ(store.voted(), false);
  EXPECT_EQ(store.voted(true), true);
  EXPECT_EQ(store.voted(), true);

  EXPECT_EQ(store.voted_for(), (uint32_t)0);
  EXPECT_EQ(store.voted_for(1), true);
  EXPECT_EQ(store.voted_for(), (uint32_t)1);

  EXPECT_EQ(store.logs_size(), (uint64_t)0);
  auto command = akit::failover::foros::Command::make_shared(
      std::initializer_list<uint8_t>{'a'});
  EXPECT_EQ(store.push_log(akit::failover::foros::raft::LogEntry::make_shared(
                0, 10, command)),
            true);

  EXPECT_EQ(store.logs_size(), (uint64_t)1);
  auto log = store.log(0);
  EXPECT_EQ(log, store.log());
  EXPECT_EQ(log->id_, (uint64_t)0);
  EXPECT_EQ(log->term_, (uint64_t)10);
  EXPECT_EQ(log->command_->data()[0], 'a');
  EXPECT_EQ(log->command_->data().size(), (std::size_t)1);
  EXPECT_EQ(store.revert_log(1), false);
  EXPECT_EQ(store.revert_log(0), true);
  EXPECT_EQ(store.logs_size(), (uint64_t)0);
  EXPECT_EQ(store.push_log(akit::failover::foros::raft::LogEntry::make_shared(
                0, 10, command)),
            true);
}

TEST_F(TestRaft, TestContextStoreWithInitialData) {
  auto store = akit::failover::foros::raft::ContextStore(kStorePath, logger_);

  EXPECT_EQ(store.current_term(), (uint64_t)1);
  EXPECT_EQ(store.voted(), true);
  EXPECT_EQ(store.voted_for(), (uint32_t)1);

  EXPECT_EQ(store.logs_size(), (uint64_t)1);
  auto log = store.log(0);
  EXPECT_EQ(log, store.log());
  EXPECT_EQ(log->id_, (uint64_t)0);
  EXPECT_EQ(log->term_, (uint64_t)10);
  EXPECT_EQ(log->command_->data()[0], 'a');
  EXPECT_EQ(log->command_->data().size(), sizeof(uint8_t));
}

TEST_F(TestRaft, TestContext) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto node = rclcpp::Node(kClusterName + std::to_string(kNodeId));
  auto context = akit::failover::foros::raft::Context(
      kClusterName, kNodeId, node.get_node_base_interface(),
      node.get_node_graph_interface(), node.get_node_services_interface(),
      node.get_node_timers_interface(), node.get_node_clock_interface(), 15000,
      20000, kTempPath, logger_);
  TestStateMachine state_machine;
  context.initialize(kClusterIds, &state_machine);

  EXPECT_EQ(context.get_node_name(),
            std::string(kClusterName + std::to_string(kNodeId)));

  auto term = context.get_term();
  context.increase_term();
  EXPECT_EQ(context.get_term(), term + 1);
  EXPECT_EQ(context.get_commands_size(), (uint64_t)0);

  bool response_callback_invoked = false;
  akit::failover::foros::CommandCommitResponseCallback response_callback =
      [&](akit::failover::foros::CommandCommitResponseSharedFuture future) {
        auto response = future.get();
        RCLCPP_ERROR(logger_, "commit result: %d", response->result());
      };

  auto future =
      context.commit_command(akit::failover::foros::Command::make_shared(
                                 std::initializer_list<uint8_t>{'a'}),
                             response_callback);
  EXPECT_EQ(context.get_commands_size(), (uint64_t)1);

  context.register_on_committed(
      [&](uint64_t id, akit::failover::foros::Command::SharedPtr command) {});
  context.register_on_reverted([&](uint64_t id) {});
}
