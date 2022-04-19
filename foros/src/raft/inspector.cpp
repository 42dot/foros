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

#include "inspector.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace akit {
namespace failover {
namespace foros {
namespace raft {

Inspector::Inspector(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
    std::function<void(foros_msgs::msg::Inspector::SharedPtr msg)>
        message_request_callback)
    : message_request_callback_(message_request_callback) {
  if (message_request_callback_ == nullptr || !is_enabled()) {
    return;
  }

  auto period = get_period();
  if (period <= 0) {
    period = default_period_;
  }

  timer_ = rclcpp::GenericTimer<rclcpp::VoidCallbackType>::make_shared(
      node_clock->get_clock(),
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::duration<double>(period)),
      [this]() {
        auto msg = std::make_shared<foros_msgs::msg::Inspector>();
        message_request_callback_(msg);
        inspector_publisher_->publish(*msg);
      },
      node_base->get_context());
  node_timers->add_timer(timer_, nullptr);

  initialize_publisher(node_topics);
}

Inspector::~Inspector() {
  if (timer_ != nullptr) {
    timer_->cancel();
    timer_->reset();
  }
}

void Inspector::initialize_publisher(
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics) {
  inspector_publisher_ = std::dynamic_pointer_cast<
      rclcpp::Publisher<foros_msgs::msg::Inspector>>(
      node_topics->create_publisher(
          foros_msgs::msg::Inspector::TOPIC_NAME,
          rclcpp::create_publisher_factory<
              foros_msgs::msg::Inspector, std::allocator<void>,
              rclcpp::Publisher<foros_msgs::msg::Inspector>>(
              rclcpp::PublisherOptionsWithAllocator<std::allocator<void>>()),
          rclcpp::QoS(0)));

  node_topics->add_publisher(inspector_publisher_, nullptr);
}

bool Inspector::is_enabled() {
  char *value = std::getenv(env_var_enable_);
  if (value == nullptr) {
    return false;
  }

  return (strcmp(value, "1") == 0);
}

double Inspector::get_period() {
  char *value = std::getenv(env_var_period_);
  if (value == nullptr) {
    return 0;
  }

  return std::atof(value);
}

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit
