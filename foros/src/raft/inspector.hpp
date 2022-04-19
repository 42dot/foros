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

#ifndef AKIT_FAILOVER_FOROS_RAFT_INSPECTOR_HPP_
#define AKIT_FAILOVER_FOROS_RAFT_INSPECTOR_HPP_

#include <foros_msgs/msg/inspector.hpp>
#include <rclcpp/rclcpp.hpp>

#include <functional>
#include <string>

#include "raft/context_store.hpp"

namespace akit {
namespace failover {
namespace foros {
namespace raft {

class Inspector {
 public:
  Inspector(rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
            rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
            rclcpp::node_interfaces::NodeTimersInterface::SharedPtr node_timers,
            rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock,
            std::function<void(foros_msgs::msg::Inspector::SharedPtr msg)>
                message_request_callback);

  ~Inspector();

 private:
  void initialize_publisher(
      rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics);

  bool is_enabled();
  double get_period();

  const char *env_var_period_ = "FOROS_INSPECTOR_PERIOD";
  const char *env_var_enable_ = "FOROS_INSPECTOR";
  const double default_period_ = 1.0;

  rclcpp::Publisher<foros_msgs::msg::Inspector>::SharedPtr inspector_publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::function<void(foros_msgs::msg::Inspector::SharedPtr msg)>
      message_request_callback_;
};

}  // namespace raft
}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_RAFT_INSPECTOR_HPP_
