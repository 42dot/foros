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

#include "node_cluster_impl.hpp"

#include <rclcpp/node_interfaces/node_base.hpp>

#include <memory>
#include <string>

namespace akit {
namespace failsafe {
namespace fsros {

NodeClusterImpl::NodeClusterImpl(const std::string &node_name,
                                 const std::string &node_namespace,
                                 const rclcpp::NodeOptions &options)
    : node_base_(new rclcpp::node_interfaces::NodeBase(
          node_name, node_namespace, options.context(),
          *(options.get_rcl_node_options()), options.use_intra_process_comms(),
          options.enable_topic_statistics())),
      raft_fsm_(std::make_unique<StateMachine>()) {}

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit
