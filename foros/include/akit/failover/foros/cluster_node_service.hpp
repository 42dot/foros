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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_SERVICE_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_SERVICE_HPP_

#include <rclcpp/service.hpp>

#include <memory>
#include <string>
#include <utility>

#include "cluster_node_lifecycle_interface.hpp"

namespace akit {
namespace failover {
namespace foros {

/// Specialized ROS service for a clustered node.
/**
 * This service accepts a request from a client only if the clustered node is
 * active.
 */
template <typename ServiceT>
class ClusterNodeService : public rclcpp::Service<ServiceT> {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(ClusterNodeService)

  /// Create ClusterNodeService.
  /**
   * The constructor for a ClsuterNodeService is almost never called directly.
   * Instead, services should be instantiated through the function
   * ClusterNode::craete_service().
   *
   * \param[in] node_handle NodeBaseInterface pointer.
   * \param[in] service_name Name of the topic to publish to.
   * \param[in] any_callback Callback to call when a client request is received.
   * \param[in] service_options options for the subscription.
   */
  ClusterNodeService(std::shared_ptr<rcl_node_t> node_handle,
                     const std::string &service_name,
                     rclcpp::AnyServiceCallback<ServiceT> any_callback,
                     rcl_service_options_t &service_options)
      : rclcpp::Service<ServiceT>(node_handle, service_name, any_callback,
                                  service_options),
        node_lifecycle_interface_(nullptr),
        logger_(rclcpp::get_logger("ClusterNodeService")) {}

  ~ClusterNodeService() {}

  /// Handle a request of service.
  /**
   * The function checks whether the node is active or not and forwards the
   * request to the actual rclcpp::Service class.
   *
   * \param[in] request_header a header of the request.
   * \param[in] request a handle of the request.
   */
  void handle_request(std::shared_ptr<rmw_request_id_t> request_header,
                      std::shared_ptr<void> request) override {
    if (node_lifecycle_interface_ != nullptr &&
        !node_lifecycle_interface_->is_activated()) {
      // ignore handle request when service is not activated
      return;
    }
    rclcpp::Service<ServiceT>::handle_request(request_header, request);
  }

  /// Set the node lifecycle interface to check whether the node is active or
  /// not.
  /**
   * \param[in] interface node interface.
   */
  void set_node_lifecycle_interface(ClusterNodeLifecycleInterface *interface) {
    node_lifecycle_interface_ = interface;
  }

 private:
  ClusterNodeLifecycleInterface *node_lifecycle_interface_;
  rclcpp::Logger logger_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_SERVICE_HPP_
