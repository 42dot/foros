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

#include "raft/event/event.hpp"
#include "raft/state/state_type.hpp"
#include "raft/state_machine.hpp"

namespace fsros = akit::failsafe::fsros;

class TestNodeCluster : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}

  void SetUp() { state_machine_ = std::make_shared<fsros::StateMachine>(); }

  void TearDown() { state_machine_.reset(); }

  std::shared_ptr<fsros::StateMachine> state_machine_;
};

// Test in standby state
TEST_F(TestNodeCluster, TestStandbyStateInit) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);

  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}

TEST_F(TestNodeCluster, TestStandbyStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);

  state_machine_->Handle(fsros::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);

  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);

  state_machine_->Handle(fsros::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);

  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);

  state_machine_->Handle(fsros::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

// Test in follower state
TEST_F(TestNodeCluster, TestFollowerStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);

  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}

TEST_F(TestNodeCluster, TestFollowerStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);

  state_machine_->Handle(fsros::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestFollowerStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);

  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
}

TEST_F(TestNodeCluster, TestFollowerStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);

  state_machine_->Handle(fsros::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}

TEST_F(TestNodeCluster, TestFollowerStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);

  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}

TEST_F(TestNodeCluster, TestFollowerStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);

  state_machine_->Handle(fsros::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}

// Test in candidate state
TEST_F(TestNodeCluster, TestCandidateStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);

  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
}

TEST_F(TestNodeCluster, TestCandidateStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);

  state_machine_->Handle(fsros::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestCandidateStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);

  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
}

TEST_F(TestNodeCluster, TestCandidateStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);

  state_machine_->Handle(fsros::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
}

TEST_F(TestNodeCluster, TestCandidateStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);

  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);
}

TEST_F(TestNodeCluster, TestCandidateStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);

  state_machine_->Handle(fsros::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}

// Test in leader state
TEST_F(TestNodeCluster, TestLeaderStateStartedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);

  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateTerminatedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);

  state_machine_->Handle(fsros::Event::kTerminated);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
}

TEST_F(TestNodeCluster, TestLeaderStateTimedoutEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);

  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateVoteReceivedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);

  state_machine_->Handle(fsros::Event::kVoteReceived);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateElectedEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);

  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kStandBy);
  state_machine_->Handle(fsros::Event::kStarted);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
  state_machine_->Handle(fsros::Event::kTimedout);
  EXPECT_TRUE(state_machine_->GetCurrentState() ==
              fsros::StateType::kCandidate);
  state_machine_->Handle(fsros::Event::kElected);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kLeader);

  state_machine_->Handle(fsros::Event::kLeaderDiscovered);
  EXPECT_TRUE(state_machine_->GetCurrentState() == fsros::StateType::kFollower);
}
