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

#include "raft/event/elected.hpp"
#include "raft/event/leadaer_discovered.hpp"
#include "raft/event/started.hpp"
#include "raft/event/terminated.hpp"
#include "raft/event/timedout.hpp"
#include "raft/event/vote_received.hpp"
#include "raft/raft_state_machine.hpp"

namespace fsros = akit::failsafe::fsros;

class TestNodeCluster : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}

  void SetUp() { raft_fsm_ = std::make_shared<fsros::RaftStateMachine>(); }

  void TearDown() { raft_fsm_.reset(); }

  std::shared_ptr<fsros::RaftStateMachine> raft_fsm_;
};

// Test in standby state
TEST_F(TestNodeCluster, TestStandbyStateInit) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateStartedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);

  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}

TEST_F(TestNodeCluster, TestStandbyStateTerminatedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);

  raft_fsm_->handle(fsros::Terminated());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateTimedoutEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);

  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateVoteReceivedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);

  raft_fsm_->handle(fsros::VoteReceived());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateElectedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);

  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestStandbyStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);

  raft_fsm_->handle(fsros::LeaderDiscovered());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

// Test in follower state
TEST_F(TestNodeCluster, TestFollowerStateStartedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);

  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}

TEST_F(TestNodeCluster, TestFollowerStateTerminatedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);

  raft_fsm_->handle(fsros::Terminated());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestFollowerStateTimedoutEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);

  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
}

TEST_F(TestNodeCluster, TestFollowerStateVoteReceivedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);

  raft_fsm_->handle(fsros::VoteReceived());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}

TEST_F(TestNodeCluster, TestFollowerStateElectedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);

  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}

TEST_F(TestNodeCluster, TestFollowerStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);

  raft_fsm_->handle(fsros::LeaderDiscovered());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}

// Test in candidate state
TEST_F(TestNodeCluster, TestCandidateStateStartedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);

  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
}

TEST_F(TestNodeCluster, TestCandidateStateTerminatedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);

  raft_fsm_->handle(fsros::Terminated());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestCandidateStateTimedoutEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);

  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
}

TEST_F(TestNodeCluster, TestCandidateStateVoteReceivedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);

  raft_fsm_->handle(fsros::VoteReceived());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
}

TEST_F(TestNodeCluster, TestCandidateStateElectedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);

  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);
}

TEST_F(TestNodeCluster, TestCandidateStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);

  raft_fsm_->handle(fsros::LeaderDiscovered());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}

// Test in leader state
TEST_F(TestNodeCluster, TestLeaderStateStartedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);

  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateTerminatedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);

  raft_fsm_->handle(fsros::Terminated());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
}

TEST_F(TestNodeCluster, TestLeaderStateTimedoutEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);

  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateVoteReceivedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);

  raft_fsm_->handle(fsros::VoteReceived());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateElectedEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);

  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);
}

TEST_F(TestNodeCluster, TestLeaderStateLeaderDiscoveredEvent) {
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kStandBy);
  raft_fsm_->handle(fsros::Started());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
  raft_fsm_->handle(fsros::Timedout());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() ==
              fsros::RaftState::kCandidate);
  raft_fsm_->handle(fsros::Elected());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kLeader);

  raft_fsm_->handle(fsros::LeaderDiscovered());
  EXPECT_TRUE(raft_fsm_->getCurrentStateIndex() == fsros::RaftState::kFollower);
}
