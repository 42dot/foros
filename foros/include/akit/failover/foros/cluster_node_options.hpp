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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_OPTIONS_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_OPTIONS_HPP_

#include <rclcpp/node_options.hpp>

#include <string>

#include "akit/failover/foros/common.hpp"

namespace akit {
namespace failover {
namespace foros {

/// Options of a clustered node
class ClusterNodeOptions : public rclcpp::NodeOptions {
 public:
  /// Create ClusterNodeOptions with default values, optionally specifying the
  /// allocator to use.
  /**
   * Default values for the cluster node options:
   *
   *   - context = rclcpp::contexts::get_global_default_context()
   *   - arguments = {}
   *   - parameter_overrides = {}
   *   - use_global_arguments = true
   *   - use_intra_process_comms = false
   *   - enable_topic_statistics = false
   *   - start_parameter_services = true
   *   - start_parameter_event_publisher = true
   *   - clock_qos = rclcpp::ClockQoS()
   *   - use_clock_thread = true
   *   - rosout_qos = rclcpp::RosoutQoS()
   *   - parameter_event_qos = rclcpp::ParameterEventQoS
   *     - with history setting and depth from rmw_qos_profile_parameter_events
   *   - parameter_event_publisher_options = rclcpp::PublisherOptionsBase
   *   - allow_undeclared_parameters = false
   *   - automatically_declare_parameters_from_overrides = false
   *   - allocator = rcl_get_default_allocator()
   *
   * Default values for the cluster node extended options:
   *   - election_timeout_min = 150ms
   *   - election_timeout_max = 300ms
   *
   * \param[in] allocator allocator to use in construction of
   *   ClusterNodeOptions.
   */
  CLUSTER_NODE_PUBLIC
  explicit ClusterNodeOptions(
      rcl_allocator_t allocator = rcl_get_default_allocator());

  CLUSTER_NODE_PUBLIC
  virtual ~ClusterNodeOptions() = default;

  /// Get the minimum value of election timeout.
  /**
   * \return The minimum value of election timeout.
   */
  CLUSTER_NODE_PUBLIC
  unsigned int election_timeout_min() const;

  /// Set the minimum value of election timeout.
  /**
   * \param min the minimum value of election timeout.
   * \return The reference of this instance
   */
  CLUSTER_NODE_PUBLIC
  ClusterNodeOptions &election_timeout_min(unsigned int min);

  /// Return the maximum value of election timeout.
  /**
   * \return The maximum value of election timeout.
   */
  CLUSTER_NODE_PUBLIC
  unsigned int election_timeout_max() const;

  /// Set the maximum value of election timeout.
  /**
   * \param max the maximum value of election timeout.
   * \return The reference of this instance.
   */
  CLUSTER_NODE_PUBLIC
  ClusterNodeOptions &election_timeout_max(unsigned int max);

  /// Return the temp directory. the files in the temp directory must be cleared
  /// every boot. ex) /tmp in linux.
  CLUSTER_NODE_PUBLIC
  std::string temp_directory() const;

  /// Set the temp directory.
  /**
   * \param directory the share directory.
   * \return The reference of this instance.
   */
  CLUSTER_NODE_PUBLIC
  ClusterNodeOptions &temp_directory(std::string &directory);

 private:
  unsigned int election_timeout_min_;
  unsigned int election_timeout_max_;
  std::string temp_directory_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_OPTIONS_HPP_
