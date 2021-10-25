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

#ifndef AKIT_FAILSAFE_FSROS_CLUSTER_NODE_HPP_
#define AKIT_FAILSAFE_FSROS_CLUSTER_NODE_HPP_

#include <rclcpp/callback_group.hpp>
#include <rclcpp/create_publisher.hpp>
#include <rclcpp/create_subscription.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_timers.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/node_options.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "akit/failsafe/fsros/cluster_node_interface.hpp"
#include "akit/failsafe/fsros/cluster_node_publisher.hpp"
#include "akit/failsafe/fsros/common.hpp"

namespace akit {
namespace failsafe {
namespace fsros {

class ClusterNodeImpl;

/// ClusterNode for creating a clustered node.
/**
 * has cluster node interfaces.
 */
class ClusterNode : public ClusterNodeInterface {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(ClusterNode)

  /// Create a new cluster node with the specified name.
  /**
   * \param[in] node_name Name of the node.
   * \param[in] namespace_ Namespace of the node.
   * \param[in] options Additional options to control creation of the node.
   */
  CLUSTER_NODE_PUBLIC
  explicit ClusterNode(
      const std::string &node_name, const std::string &node_namespace = "",
      const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

  CLUSTER_NODE_PUBLIC
  virtual ~ClusterNode();

  /// Get the name of the node.
  /**
   * \return The name of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_name() const;

  /// Get the namespace of the node
  /**
   * \return The namespace of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_namespace() const;

  /// Get the logger of the node.
  /**
   * \return The logger of the node.
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Logger get_logger() const;

  /// Create and return a callback group.
  /**
   * \param[in] group_type callback group type to create by this method.
   * \param[in] automatically_add_to_executor_with_node A boolean that
   *   determines whether a callback group is automatically added to an executor
   *   with the node with which it is associated.
   * \return a callback group
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::CallbackGroup::SharedPtr create_callback_group(
      rclcpp::CallbackGroupType group_type,
      bool automatically_add_to_executor_with_node = true);

  /// Return the list of callback groups in the node.
  /**
   * \return list of callback groups in the node.
   */
  CLUSTER_NODE_PUBLIC
  const std::vector<rclcpp::CallbackGroup::WeakPtr> &get_callback_groups()
      const;

  /// Create and return a Publisher.
  /**
   * \param[in] topic_name The topic for this publisher to publish on.
   * \param[in] qos The Quality of Service settings for this publisher.
   * \param[in] options The publisher options for this publisher.
   * \return Shared pointer to the created cluster node publisher.
   */
  template <typename MessageT, typename AllocatorT = std::allocator<void>>
  std::shared_ptr<ClusterNodePublisher<MessageT, AllocatorT>> create_publisher(
      const std::string &topic_name, const rclcpp::QoS &qos,
      const rclcpp::PublisherOptionsWithAllocator<AllocatorT> &options =
          (rclcpp::PublisherOptionsWithAllocator<AllocatorT>())) {
    return rclcpp::create_publisher<MessageT, AllocatorT,
                                    ClusterNodePublisher<MessageT, AllocatorT>>(
        *this, topic_name, qos, options);
  }

  /// Create and return a Subscription.
  /**
   * \param[in] topic_name The topic to subscribe on.
   * \param[in] callback The user-defined callback function.
   * \param[in] qos The quality of service for this subscription.
   * \param[in] options The subscription options for this subscription.
   * \param[in] msg_mem_strat The message memory strategy to use for allocating
   * messages. \return Shared pointer to the created subscription.
   */
  template <typename MessageT, typename CallbackT,
            typename AllocatorT = std::allocator<void>,
            typename CallbackMessageT = typename rclcpp::subscription_traits::
                has_message_type<CallbackT>::type,
            typename SubscriptionT = rclcpp::Subscription<MessageT, AllocatorT>,
            typename MessageMemoryStrategyT = rclcpp::message_memory_strategy::
                MessageMemoryStrategy<CallbackMessageT, AllocatorT>>
  std::shared_ptr<SubscriptionT> create_subscription(
      const std::string &topic_name, const rclcpp::QoS &qos,
      CallbackT &&callback,
      const rclcpp::SubscriptionOptionsWithAllocator<AllocatorT> &options =
          rclcpp::SubscriptionOptionsWithAllocator<AllocatorT>(),
      typename MessageMemoryStrategyT::SharedPtr msg_mem_strat =
          (MessageMemoryStrategyT::create_default())) {
    {
      return rclcpp::create_subscription<MessageT>(
          *this, topic_name, qos, std::forward<CallbackT>(callback), options,
          msg_mem_strat);
    }
  }

  /// Create a timer.
  /**
   * \param[in] period Time interval between triggers of the callback.
   * \param[in] callback User-defined callback function.
   * \param[in] group Callback group to execute this timer's callback in.
   */
  template <typename DurationRepT = int64_t, typename DurationT = std::milli,
            typename CallbackT>
  typename rclcpp::WallTimer<CallbackT>::SharedPtr create_wall_timer(
      std::chrono::duration<DurationRepT, DurationT> period, CallbackT callback,
      rclcpp::CallbackGroup::SharedPtr group = nullptr) {
    auto timer = rclcpp::WallTimer<CallbackT>::make_shared(
        std::chrono::duration_cast<std::chrono::nanoseconds>(period),
        std::move(callback), this->node_base_->get_context());
    node_timers_->add_timer(timer, group);
    return timer;
  }

  /// Return the Node's internal NodeTopicsInterface implementation.
  /**
   * \sa rclcpp::Node::get_node_topics_interface
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
  get_node_topics_interface();

 private:
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging_;
  rclcpp::node_interfaces::NodeTimers::SharedPtr node_timers_;
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics_;

  std::unique_ptr<ClusterNodeImpl> impl_;
};

}  // namespace fsros
}  // namespace failsafe
}  // namespace akit

#endif  // AKIT_FAILSAFE_FSROS_CLUSTER_NODE_HPP_
