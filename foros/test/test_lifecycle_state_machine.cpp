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

#include "lifecycle/event.hpp"
#include "lifecycle/state_machine.hpp"
#include "lifecycle/state_type.hpp"

namespace lifecycle = akit::failover::foros::lifecycle;

class TestLifecycleStateMachine : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}

  void SetUp() {
    auto logger = rclcpp::get_logger("test_lifecycle_state_machine");
    state_machine_ = std::make_shared<lifecycle::StateMachine>(logger);
  }

  void TearDown() { state_machine_.reset(); }

  std::shared_ptr<lifecycle::StateMachine> state_machine_;
};

// Test in inactive state
TEST_F(TestLifecycleStateMachine, TestInactiveStateInit) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
}

TEST_F(TestLifecycleStateMachine, TestInactiveStateActivateEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);

  state_machine_->handle(lifecycle::Event::kActivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kActive);
}

TEST_F(TestLifecycleStateMachine, TestInactiveStateDeactivateEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);

  state_machine_->handle(lifecycle::Event::kDeactivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
}

TEST_F(TestLifecycleStateMachine, TestInactiveStateStandbyEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);

  state_machine_->handle(lifecycle::Event::kStandby);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kStandby);
}

// Test in active state
TEST_F(TestLifecycleStateMachine, TestActiveStateActivateEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
  state_machine_->handle(lifecycle::Event::kActivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kActive);

  state_machine_->handle(lifecycle::Event::kActivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kActive);
}

TEST_F(TestLifecycleStateMachine, TestActiveStateDeactivateEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
  state_machine_->handle(lifecycle::Event::kActivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kActive);

  state_machine_->handle(lifecycle::Event::kDeactivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
}

TEST_F(TestLifecycleStateMachine, TestActiveStateStandbyEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
  state_machine_->handle(lifecycle::Event::kActivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kActive);

  state_machine_->handle(lifecycle::Event::kStandby);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kStandby);
}

// Test in standby state
TEST_F(TestLifecycleStateMachine, TestStandbyStateActivateEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
  state_machine_->handle(lifecycle::Event::kStandby);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kStandby);

  state_machine_->handle(lifecycle::Event::kActivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kActive);
}

TEST_F(TestLifecycleStateMachine, TestStandbyStateDeactivateEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
  state_machine_->handle(lifecycle::Event::kStandby);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kStandby);

  state_machine_->handle(lifecycle::Event::kDeactivate);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
}

TEST_F(TestLifecycleStateMachine, TestStandbyStateStandbyEvent) {
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kInactive);
  state_machine_->handle(lifecycle::Event::kStandby);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kStandby);

  state_machine_->handle(lifecycle::Event::kStandby);
  EXPECT_TRUE(state_machine_->get_current_state_type() ==
              lifecycle::StateType::kStandby);
}
