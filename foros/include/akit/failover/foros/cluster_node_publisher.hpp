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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_PUBLISHER_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_PUBLISHER_HPP_

#include <rclcpp/logging.hpp>
#include <rclcpp/publisher.hpp>

#include <memory>
#include <string>
#include <utility>

#include "akit/failover/foros/cluster_node_lifecycle_interface.hpp"

namespace akit {
namespace failover {
namespace foros {

/// Specialized ROS publisher for a clustered node.
/**
 * This publisher publishes a message only if the clustered node is active.
 */
template <typename MessageT, typename Alloc = std::allocator<void>>
class ClusterNodePublisher : public rclcpp::Publisher<MessageT, Alloc> {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(ClusterNodePublisher)

  using MessageAllocTraits = rclcpp::allocator::AllocRebind<MessageT, Alloc>;
  using MessageAlloc = typename MessageAllocTraits::allocator_type;
  using MessageDeleter = rclcpp::allocator::Deleter<MessageAlloc, MessageT>;
  using MessageUniquePtr = std::unique_ptr<MessageT, MessageDeleter>;

  /// Create ClusterNodePublisher.
  /**
   * The constructor for a ClsuterNodePublisher is almost never called
   * directly. Instead, publisher should be instantiated through the function
   * ClusterNode::craete_publisher().
   *
   * \param[in] node_base Handle of node base interface.
   * \param[in] topic Name of the topic.
   * \param[in] qos QoS settings.
   * \param[in] options Options of the publisher.
   */
  ClusterNodePublisher(
      rclcpp::node_interfaces::NodeBaseInterface* node_base,
      const std::string& topic, const rclcpp::QoS& qos,
      const rclcpp::PublisherOptionsWithAllocator<Alloc>& options)
      : rclcpp::Publisher<MessageT, Alloc>(node_base, topic, qos, options),
        node_lifecycle_interface_(nullptr),
        logger_(rclcpp::get_logger("ClusterNodePublisher")) {}

  ~ClusterNodePublisher() {}

  /// Publish a message.
  /**
   * The publish function checks whether the node is active or not and forwards
   * the message to the actual rclcpp::Publisher class.
   *
   * \param[in] msg a message to publish.
   */
  void publish(std::unique_ptr<MessageT, MessageDeleter> msg) override {
    if (node_lifecycle_interface_ != nullptr &&
        !node_lifecycle_interface_->is_activated()) {
      // ignore publish request when publisher is not activated
      return;
    }
    rclcpp::Publisher<MessageT, Alloc>::publish(std::move(msg));
  }

  /// Publish a message.
  /**
   * The publish function checks whether the node is active or not and forwards
   * the message to the actual rclcpp::Publisher class.
   *
   * \param[in] msg a message to publish.
   */
  void publish(const MessageT& msg) override {
    if (node_lifecycle_interface_ != nullptr &&
        !node_lifecycle_interface_->is_activated()) {
      // ignore publish request when publisher is not activated
      return;
    }
    rclcpp::Publisher<MessageT, Alloc>::publish(msg);
  }

  /// Set the node lifecycle interface to check whether the node is active or
  /// not.
  /**
   * \param[in] interface node interface.
   */
  void set_node_lifecycle_interface(ClusterNodeLifecycleInterface* interface) {
    node_lifecycle_interface_ = interface;
  }

 private:
  ClusterNodeLifecycleInterface* node_lifecycle_interface_;
  rclcpp::Logger logger_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_PUBLISHER_HPP_
