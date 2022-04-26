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
#include "colors.hpp"
#include "node_info.hpp"

namespace akit {
namespace failover {
namespace foros_inspector {

class Inspector : public rclcpp::Node {
 public:
  Inspector();
  virtual ~Inspector();

 private:
  static const char *kNodeName;

  void initialize_refresh_timer();
  void reset_refresh_timer();

  void intialize_screen();
  void terminate_screen();
  void refresh_screen();
  void update_screen();
  void add_summary();
  void add_cluster_item(std::shared_ptr<ClusterInfo> cluster);
  void add_details(std::shared_ptr<ClusterInfo> cluster);
  void add_node_item(std::shared_ptr<NodeInfo> node);
  void add_newline();
  void add_title(const std::string &str);
  void add_subtitle(const std::string &str);
  void add_string(const char *str, const uint32_t size);
  void add_string(const char *str, const uint32_t size, Colors color);
  void add_bold_string(const char *str, const uint32_t size, Colors color);
  void add_number(const uint32_t num, const uint32_t size);
  void add_number(const uint64_t num, const uint32_t size);
  void add_error(const char *str, const uint32_t size);
  void add_separater();
  void add_state_name(const uint8_t state);

  bool is_outdated(rclcpp::Time time);
  std::shared_ptr<ClusterInfo> get_cluster_info(const std::string &name);
  std::shared_ptr<NodeInfo> get_node_info(std::shared_ptr<ClusterInfo> cluster,
                                          const uint32_t id);
  void update_cluster_info();
  void inspector_message_received(
      const foros_msgs::msg::Inspector::SharedPtr msg);

  double get_period();

  const char *divider_ =
      "------------------------------------------------------------------------"
      "-----------------------------------------------------------------------";
  const uint32_t xlarge_column_ = 20;
  const uint32_t large_column_ = 15;
  const uint32_t medium_column_ = 10;
  const uint32_t small_column_ = 5;
  uint32_t name_column_;

  const char *env_var_period_ = "FOROS_INSPECTOR_PERIOD";
  const double default_period_ = 1.0;
  double period_;

  std::unordered_map<std::string, std::shared_ptr<ClusterInfo>> clusters_;
  rclcpp::Subscription<foros_msgs::msg::Inspector>::SharedPtr subscriber_;
  rclcpp::TimerBase::SharedPtr refresh_timer_;

  WINDOW *window_;
};

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_INSPECTOR_INSPECTOR_HPP_
