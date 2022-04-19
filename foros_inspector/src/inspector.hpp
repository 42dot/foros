/*
 * Copyright (c) 2022 42dot All rights reserved.
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

#ifndef AKIT_FAILOVER_FOROS_INSPECTOR_INSPECTOR_HPP_
#define AKIT_FAILOVER_FOROS_INSPECTOR_INSPECTOR_HPP_

#include <foros_msgs/msg/inspector.hpp>
#include <ncurses.h>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <string>
#include <unordered_map>

#include "cluster_info.hpp"
#include "node_info.hpp"

namespace akit {
namespace failover {
namespace foros_inspector {

class Inspector : public rclcpp::Node {
 public:
  Inspector();
  virtual ~Inspector();

 private:
  static const std::string kNodeName;

  void initialize_refresh_timer();
  void reset_refresh_timer();

  void intialize_screen();
  void terminate_screen();
  void refresh_screen();
  void update_screen();
  void add_summary();
  void add_details(std::shared_ptr<ClusterInfo> cluster);
  void add_padding();
  void add_title(const std::string &str);
  void add_subtitle(const std::string &str);
  const char *get_state_name(const uint8_t state);

  bool is_outdated(rclcpp::Time time);
  std::shared_ptr<ClusterInfo> get_cluster_info(const std::string &name,
                                                const uint32_t size);
  std::shared_ptr<NodeInfo> get_node_info(std::shared_ptr<ClusterInfo> cluster,
                                          const uint32_t id);
  void remove_outdated_cluster_info();
  void inspector_message_received(
      const foros_msgs::msg::Inspector::SharedPtr msg);

  const int32_t large_column_ = 15;
  const int32_t medium_column_ = 12;
  const char *divider_ =
      "------------------------------------------------------------------------"
      "-----------------------------------------------------------------------";

  std::unordered_map<std::string, std::shared_ptr<ClusterInfo>> clusters_;
  rclcpp::Subscription<foros_msgs::msg::Inspector>::SharedPtr subscriber_;
  rclcpp::TimerBase::SharedPtr refresh_timer_;

  WINDOW *pad;
};

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_INSPECTOR_INSPECTOR_HPP_