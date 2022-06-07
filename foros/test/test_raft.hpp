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
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node.hpp"
#include "common/node_util.hpp"
#include "raft/context.hpp"
#include "raft/context_store.hpp"
#include "raft/state_machine.hpp"
#include "raft/state_machine_interface.hpp"

class TestRaft : public ::testing::Test {
 protected:
  static void SetUpTestCase();
  static void TearDownTestCase();

  const std::string kTempPath = "/tmp";
  const std::string kStorePath = "/tmp/foros_test_cluster0";
  const std::string kInvalidStorePath = "@/3kj1;tmp/abc/def/adf01-_";
  const char* kClusterName = "test_cluster";
  const uint32_t kNodeId = 0;
  const uint32_t kOtherNodeId = 1;
  const std::vector<uint32_t> kClusterIds = std::initializer_list<uint32_t>{0};
  const std::vector<uint32_t> kClusterIds2 =
      std::initializer_list<uint32_t>{0, 1};
  const uint8_t kTestData = 'a';
  const uint64_t kCurrentTerm = 10;
  const uint32_t kVotedFor = 1;
  const uint64_t kMaxCommitSize = 3;
  const unsigned int kElectionTimeoutMin = 15000;
  const unsigned int kElectionTimeoutMax = 20000;
  rclcpp::Logger logger_ = rclcpp::get_logger("test_raft");
};

// state machine interface
class MockStateMachineInterface
    : public akit::failover::foros::raft::StateMachineInterface {
 public:
  MOCK_METHOD(void, on_election_timedout, (), (override));
  MOCK_METHOD(void, on_new_term_received, (), (override));
  MOCK_METHOD(void, on_elected, (), (override));
  MOCK_METHOD(void, on_broadcast_timedout, (), (override));
  MOCK_METHOD(void, on_leader_discovered, (), (override));
  MOCK_METHOD(bool, is_leader, (), (override));

  akit::failover::foros::raft::StateType get_current_state() override {
    return akit::failover::foros::raft::StateType::kStandby;
  }
};

class TestContext : public akit::failover::foros::raft::Context {
 public:
  TestContext(const std::string& cluster_name, const uint32_t node_id,
              rclcpp::Node::SharedPtr node,
              const unsigned int election_timeout_min,
              const unsigned int election_timeout_max,
              const std::string& temp_directory, rclcpp::Logger& logger)
      : akit::failover::foros::raft::Context(
            cluster_name, node_id, node->get_node_base_interface(),
            node->get_node_graph_interface(),
            node->get_node_services_interface(),
            node->get_node_topics_interface(),
            node->get_node_timers_interface(), node->get_node_clock_interface(),
            election_timeout_min, election_timeout_max, temp_directory, logger),
        cluster_name_(cluster_name),
        node_id_(node_id) {
    rcl_client_options_t client_options = rcl_client_get_default_options();
    client_options.qos = rmw_qos_profile_services_default;

    append_entries_ =
        rclcpp::Client<foros_msgs::srv::AppendEntries>::make_shared(
            node->get_node_base_interface().get(),
            node->get_node_graph_interface(),
            akit::failover::foros::NodeUtil::get_service_name(
                cluster_name, node_id,
                akit::failover::foros::NodeUtil::kAppendEntriesServiceName),
            client_options);
  }

  rclcpp::Client<foros_msgs::srv::AppendEntries>::SharedFuture
  send_append_entries_to_me(uint64_t term, uint32_t leader_id,
                            uint64_t leader_commit, uint64_t prev_log_index,
                            uint64_t prev_log_term,
                            std::vector<uint8_t> entries) {
    auto request = std::make_shared<foros_msgs::srv::AppendEntries::Request>();
    request->term = term;
    request->leader_id = leader_id;
    request->leader_commit = leader_commit;
    request->prev_log_index = prev_log_index;
    request->prev_log_term = prev_log_term;
    request->entries = entries;
    return append_entries_->async_send_request(request);
  }

 private:
  rclcpp::Client<foros_msgs::srv::AppendEntries>::SharedPtr append_entries_;
  std::string cluster_name_;
  uint64_t node_id_;
};

TEST_F(TestRaft, TestContextStore) {
  // Clear temp directory to store logs
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto store = akit::failover::foros::raft::ContextStore(kStorePath, logger_);

  // test current term
  EXPECT_EQ(store.current_term(), (uint64_t)0);
  EXPECT_EQ(store.current_term(kCurrentTerm), true);
  EXPECT_EQ(store.current_term(), kCurrentTerm);

  // test voted
  EXPECT_EQ(store.voted(), false);
  EXPECT_EQ(store.voted(true), true);
  EXPECT_EQ(store.voted(), true);

  // test voted_for
  EXPECT_EQ(store.voted_for(), (uint32_t)0);
  EXPECT_EQ(store.voted_for(kVotedFor), true);
  EXPECT_EQ(store.voted_for(), kVotedFor);

  // test logs
  EXPECT_EQ(store.logs_size(), (uint64_t)0);
  auto command = akit::failover::foros::Command::make_shared(
      std::initializer_list<uint8_t>{kTestData});
  EXPECT_EQ(store.push_log(akit::failover::foros::raft::LogEntry::make_shared(
                0, kCurrentTerm, command)),
            true);
  EXPECT_EQ(store.logs_size(), (uint64_t)1);

  auto log = store.log(0);
  EXPECT_EQ(log, store.log());
  EXPECT_EQ(log->id_, (uint64_t)0);
  EXPECT_EQ(log->term_, (uint64_t)10);
  EXPECT_EQ(log->command_->data()[0], kTestData);
  EXPECT_EQ(log->command_->data().size(), (std::size_t)1);

  EXPECT_EQ(store.revert_log(1), false);
  EXPECT_EQ(store.revert_log(0), true);
  EXPECT_EQ(store.logs_size(), (uint64_t)0);

  uint64_t i;
  for (i = 0; i < kMaxCommitSize; i++) {
    EXPECT_EQ(store.push_log(akit::failover::foros::raft::LogEntry::make_shared(
                  i, kCurrentTerm, command)),
              true);
  }
  // push log with invalid ID
  EXPECT_EQ(store.push_log(akit::failover::foros::raft::LogEntry::make_shared(
                i + 1, kCurrentTerm, command)),
            false);
}

TEST_F(TestRaft, TestContextStoreWithInitialData) {
  auto store = akit::failover::foros::raft::ContextStore(kStorePath, logger_);

  EXPECT_EQ(store.current_term(), kCurrentTerm);
  EXPECT_EQ(store.voted(), true);
  EXPECT_EQ(store.voted_for(), kVotedFor);

  EXPECT_EQ(store.logs_size(), kMaxCommitSize);
  auto log = store.log(kMaxCommitSize - 1);
  EXPECT_EQ(log, store.log());
  EXPECT_EQ(log->id_, kMaxCommitSize - 1);
  EXPECT_EQ(log->term_, kCurrentTerm);
  EXPECT_EQ(log->command_->data()[0], kTestData);
  EXPECT_EQ(log->command_->data().size(), sizeof(kTestData));
}

TEST_F(TestRaft, TestContextStoreWithInvalidPath) {
  auto store =
      akit::failover::foros::raft::ContextStore(kInvalidStorePath, logger_);

  // test current term
  EXPECT_EQ(store.current_term(), (uint64_t)0);
  EXPECT_EQ(store.current_term(kCurrentTerm), false);
  EXPECT_EQ(store.current_term(), kCurrentTerm);

  // test voted
  EXPECT_EQ(store.voted(), false);
  EXPECT_EQ(store.voted(true), false);
  EXPECT_EQ(store.voted(), true);

  // test voted_for
  EXPECT_EQ(store.voted_for(), (uint32_t)0);
  EXPECT_EQ(store.voted_for(kVotedFor), false);
  EXPECT_EQ(store.voted_for(), kVotedFor);
}

TEST_F(TestRaft, TestContextTermMethods) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto node = rclcpp::Node(kClusterName + std::to_string(kNodeId));
  auto context = akit::failover::foros::raft::Context(
      kClusterName, kNodeId, node.get_node_base_interface(),
      node.get_node_graph_interface(), node.get_node_services_interface(),
      node.get_node_topics_interface(), node.get_node_timers_interface(),
      node.get_node_clock_interface(), kElectionTimeoutMin, kElectionTimeoutMax,
      kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(true));
  context.initialize(kClusterIds, &state_machine);

  EXPECT_EQ(context.get_node_name(),
            std::string(kClusterName + std::to_string(kNodeId)));

  auto term = context.get_term();
  context.increase_term();
  EXPECT_EQ(context.get_term(), term + 1);
}

TEST_F(TestRaft, TestContextLeaderCommandCommit) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto context = TestContext(
      kClusterName, kNodeId,
      rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId)),
      kElectionTimeoutMin, kElectionTimeoutMax, kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(true));
  context.initialize(kClusterIds, &state_machine);

  testing::MockFunction<void(const uint64_t,
                             akit::failover::foros::Command::SharedPtr)>
      on_committed_callback;
  testing::MockFunction<void(const uint64_t)> on_reverted_callback;
  testing::MockFunction<void(
      akit::failover::foros::CommandCommitResponseSharedFuture)>
      on_commit_response;

  auto next_id = context.get_commands_size();
  EXPECT_EQ(next_id, uint64_t(0));

  EXPECT_CALL(on_commit_response, Call(testing::_)).WillOnce(testing::Return());
  EXPECT_CALL(on_committed_callback, Call(next_id, testing::_))
      .WillOnce(testing::Return());
  EXPECT_CALL(on_reverted_callback, Call(testing::_)).Times(0);

  context.register_on_committed(on_committed_callback.AsStdFunction());
  context.register_on_reverted(on_reverted_callback.AsStdFunction());

  EXPECT_EQ(context.get_commands_size(), (uint64_t)0);
  auto future =
      context.commit_command(akit::failover::foros::Command::make_shared(
                                 std::initializer_list<uint8_t>{kTestData}),
                             on_commit_response.AsStdFunction());

  EXPECT_EQ(context.get_commands_size(), (uint64_t)1);

  auto command = context.get_command(next_id);
  EXPECT_NE(command, nullptr);
  EXPECT_EQ(command->data()[0], kTestData);
}

TEST_F(TestRaft, TestContextNonLeaderCommandCommit) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto context = TestContext(
      kClusterName, kNodeId,
      rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId)),
      kElectionTimeoutMin, kElectionTimeoutMax, kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(false));
  context.initialize(kClusterIds, &state_machine);

  testing::MockFunction<void(const uint64_t,
                             akit::failover::foros::Command::SharedPtr)>
      on_committed_callback;
  testing::MockFunction<void(const uint64_t)> on_reverted_callback;
  testing::MockFunction<void(
      akit::failover::foros::CommandCommitResponseSharedFuture)>
      on_commit_response;

  auto next_id = context.get_commands_size();
  EXPECT_EQ(next_id, uint64_t(0));

  EXPECT_CALL(on_commit_response, Call(testing::_)).WillOnce(testing::Return());
  EXPECT_CALL(on_committed_callback, Call(next_id, testing::_)).Times(0);
  EXPECT_CALL(on_reverted_callback, Call(testing::_)).Times(0);

  context.register_on_committed(on_committed_callback.AsStdFunction());
  context.register_on_reverted(on_reverted_callback.AsStdFunction());

  EXPECT_EQ(context.get_commands_size(), (uint64_t)0);
  auto future =
      context.commit_command(akit::failover::foros::Command::make_shared(
                                 std::initializer_list<uint8_t>{kTestData}),
                             on_commit_response.AsStdFunction());

  EXPECT_EQ(context.get_commands_size(), (uint64_t)0);
}

TEST_F(TestRaft, TestContextCommandCommitPending) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }

  auto context = TestContext(
      kClusterName, kNodeId,
      rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId)),
      kElectionTimeoutMin, kElectionTimeoutMax, kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(true));
  context.initialize(kClusterIds2, &state_machine);

  testing::MockFunction<void(const uint64_t,
                             akit::failover::foros::Command::SharedPtr)>
      on_committed_callback;
  testing::MockFunction<void(const uint64_t)> on_reverted_callback;
  testing::MockFunction<void(
      akit::failover::foros::CommandCommitResponseSharedFuture)>
      on_commit_response;

  auto next_id = context.get_commands_size();
  EXPECT_EQ(next_id, uint64_t(0));

  EXPECT_CALL(on_commit_response, Call(testing::_)).Times(0);
  EXPECT_CALL(on_committed_callback, Call(next_id, testing::_)).Times(0);
  EXPECT_CALL(on_reverted_callback, Call(testing::_)).Times(0);

  context.register_on_committed(on_committed_callback.AsStdFunction());
  context.register_on_reverted(on_reverted_callback.AsStdFunction());

  EXPECT_EQ(context.get_commands_size(), (uint64_t)0);
  auto future =
      context.commit_command(akit::failover::foros::Command::make_shared(
                                 std::initializer_list<uint8_t>{kTestData}),
                             on_commit_response.AsStdFunction());

  EXPECT_EQ(context.get_commands_size(), (uint64_t)0);
}

TEST_F(TestRaft, TestContextAppendEntriesReceived) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }
  auto node = rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId));
  auto context = TestContext(kClusterName, kNodeId, node, kElectionTimeoutMin,
                             kElectionTimeoutMax, kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(true));
  EXPECT_CALL(state_machine, on_leader_discovered()).Times(3);
  context.initialize(kClusterIds2, &state_machine);

  // Pretend we received entries from other node
  uint64_t prev_index = 0;
  for (uint64_t i = 0; i < 3; i++) {
    auto future = context.send_append_entries_to_me(
        kCurrentTerm, kOtherNodeId, i, prev_index, kCurrentTerm,
        std::initializer_list<uint8_t>{kTestData});

    rclcpp::spin_until_future_complete(node, future, std::chrono::seconds(1));

    EXPECT_EQ(context.get_commands_size(), i + 1);
    auto command = context.get_command(i);
    ASSERT_NE(command, nullptr);
    EXPECT_EQ(command->data()[0], kTestData);
    prev_index = i;
  }
}

TEST_F(TestRaft, TestContextInvalidAppendEntriesReceived) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }
  auto node = rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId));
  auto context = TestContext(kClusterName, kNodeId, node, kElectionTimeoutMin,
                             kElectionTimeoutMax, kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(true));
  EXPECT_CALL(state_machine, on_leader_discovered()).Times(3);
  context.initialize(kClusterIds2, &state_machine);

  // Pretend we received entries from other node
  uint64_t prev_index = 0;
  for (uint64_t i = 1; i < 4; i++) {
    auto future = context.send_append_entries_to_me(
        kCurrentTerm, kOtherNodeId, i, prev_index, kCurrentTerm,
        std::initializer_list<uint8_t>{kTestData});

    rclcpp::spin_until_future_complete(node, future, std::chrono::seconds(1));

    EXPECT_EQ(context.get_commands_size(), (uint64_t)0);
    auto command = context.get_command(i);
    EXPECT_EQ(command, nullptr);
    prev_index = i;
  }
}

TEST_F(TestRaft, TestContextAppendEntriesReceivedForRollback) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }
  auto node = rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId));
  auto context = TestContext(kClusterName, kNodeId, node, kElectionTimeoutMin,
                             kElectionTimeoutMax, kTempPath, logger_);

  MockStateMachineInterface state_machine;
  ON_CALL(state_machine, is_leader()).WillByDefault(testing::Return(true));
  EXPECT_CALL(state_machine, on_leader_discovered()).Times(2);
  context.initialize(kClusterIds2, &state_machine);

  // Pretend we received entries from other node
  auto future = context.send_append_entries_to_me(
      kCurrentTerm, kOtherNodeId, 0, 0, kCurrentTerm,
      std::initializer_list<uint8_t>{kTestData});

  rclcpp::spin_until_future_complete(node, future, std::chrono::seconds(1));

  EXPECT_EQ(context.get_commands_size(), (uint64_t)1);
  auto command = context.get_command(0);
  ASSERT_NE(command, nullptr);
  EXPECT_EQ(command->data()[0], kTestData);

  // Send entries with invalid prev data
  future = context.send_append_entries_to_me(
      kCurrentTerm, kOtherNodeId, 1, 0, kCurrentTerm + 1,
      std::initializer_list<uint8_t>{kTestData});

  rclcpp::spin_until_future_complete(node, future, std::chrono::seconds(1));

  EXPECT_NE(context.get_commands_size(), (uint64_t)2);
  command = context.get_command(1);
  ASSERT_EQ(command, nullptr);
}

TEST_F(TestRaft, TestStateMachine) {
  try {
    std::filesystem::remove_all(kStorePath);
  } catch (const std::filesystem::filesystem_error& err) {
    RCLCPP_ERROR(logger_, "failed to remove file %s", err.what());
  }
  auto node = rclcpp::Node::make_shared(kClusterName + std::to_string(kNodeId));
  auto context = std::make_shared<TestContext>(
      kClusterName, kNodeId, node, kElectionTimeoutMin, kElectionTimeoutMax,
      kTempPath, logger_);

  MockStateMachineInterface state_machine;
  context->initialize(kClusterIds2, &state_machine);

  auto fsm =
      akit::failover::foros::raft::StateMachine(kClusterIds2, context, logger_);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kNewTermReceived);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kLeaderDiscovered);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kElected);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kLeader);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kElected);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kLeader);
  fsm.handle(akit::failover::foros::raft::Event::kLeaderDiscovered);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
  fsm.handle(akit::failover::foros::raft::Event::kStarted);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTimedout);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kCandidate);
  fsm.handle(akit::failover::foros::raft::Event::kElected);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kLeader);
  fsm.handle(akit::failover::foros::raft::Event::kNewTermReceived);
  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kFollower);
  fsm.handle(akit::failover::foros::raft::Event::kTerminated);

  EXPECT_EQ(fsm.get_current_state_type(),
            akit::failover::foros::raft::StateType::kStandby);
}
