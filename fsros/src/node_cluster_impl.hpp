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

#ifndef AKIT_FAILSAFE_FSROS_NODE_CLUSTER_IMPL_HPP_
#define AKIT_FAILSAFE_FSROS_NODE_CLUSTER_IMPL_HPP_

#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_options.hpp>

#include <string>

namespace akit {
namespace failsafe {
namespace fsros {

class NodeClusterImpl {
 public:
  explicit NodeClusterImpl(
      const std::string &node_name, const std::string &node_namespace = "",
      const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

 private:
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_NODE_CLUSTER_IMPL_HPP_
