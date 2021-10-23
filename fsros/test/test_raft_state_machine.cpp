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

#include <memory>

#include "raft/event.hpp"
#include "raft/state_machine.hpp"
#include "raft/state_type.hpp"

namespace raft = akit::failsafe::fsros::raft;

class TestRaftStateMachine : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}

  void SetUp() { state_machine_ = std::make_shared<raft::StateMachine>(); }

  void TearDown() { state_machine_.reset(); }

  std::shared_ptr<raft::StateMachine> state_machine_;
};

// Test in standby state
TEST_F(TestRaftStateMachine, TestStandbyStateInit) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestStandbyStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);

  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}

TEST_F(TestRaftStateMachine, TestStandbyStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);

  state_machine_->Handle(raft::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestStandbyStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);

  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestStandbyStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);

  state_machine_->Handle(raft::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestStandbyStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);

  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestStandbyStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);

  state_machine_->Handle(raft::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

// Test in follower state
TEST_F(TestRaftStateMachine, TestFollowerStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);

  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}

TEST_F(TestRaftStateMachine, TestFollowerStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);

  state_machine_->Handle(raft::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestFollowerStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);

  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
}

TEST_F(TestRaftStateMachine, TestFollowerStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);

  state_machine_->Handle(raft::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}

TEST_F(TestRaftStateMachine, TestFollowerStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);

  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}

TEST_F(TestRaftStateMachine, TestFollowerStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);

  state_machine_->Handle(raft::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}

// Test in candidate state
TEST_F(TestRaftStateMachine, TestCandidateStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);

  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
}

TEST_F(TestRaftStateMachine, TestCandidateStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);

  state_machine_->Handle(raft::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestCandidateStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);

  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
}

TEST_F(TestRaftStateMachine, TestCandidateStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);

  state_machine_->Handle(raft::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
}

TEST_F(TestRaftStateMachine, TestCandidateStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);

  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);
}

TEST_F(TestRaftStateMachine, TestCandidateStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);

  state_machine_->Handle(raft::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}

// Test in leader state
TEST_F(TestRaftStateMachine, TestLeaderStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);

  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);
}

TEST_F(TestRaftStateMachine, TestLeaderStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);

  state_machine_->Handle(raft::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
}

TEST_F(TestRaftStateMachine, TestLeaderStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);

  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);
}

TEST_F(TestRaftStateMachine, TestLeaderStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);

  state_machine_->Handle(raft::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);
}

TEST_F(TestRaftStateMachine, TestLeaderStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);

  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);
}

TEST_F(TestRaftStateMachine, TestLeaderStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kStandby);
  state_machine_->Handle(raft::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
  state_machine_->Handle(raft::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kCandidate);
  state_machine_->Handle(raft::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kLeader);

  state_machine_->Handle(raft::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == raft::StateType::kFollower);
}
