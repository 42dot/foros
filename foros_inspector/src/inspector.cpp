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

#include "inspector.hpp"

#include <ncurses.h>
#include <rclcpp/rclcpp.hpp>

#include <functional>
#include <memory>
#include <sstream>
#include <string>

#include "colors.hpp"

namespace akit {
namespace failover {
namespace foros_inspector {

const char *Inspector::kNodeName = "foros_inspector";

Inspector::Inspector() : rclcpp::Node(kNodeName), period_(get_period()) {
  subscriber_ = create_subscription<foros_msgs::msg::Inspector>(
      foros_msgs::msg::Inspector::TOPIC_NAME, 10,
      std::bind(&Inspector::inspector_message_received, this,
                std::placeholders::_1));
  intialize_screen();
  initialize_refresh_timer();
}

Inspector::~Inspector() { terminate_screen(); }

void Inspector::initialize_refresh_timer() {
  refresh_timer_ = rclcpp::create_timer(this, rclcpp::Clock::make_shared(),
                                        std::chrono::milliseconds(1000),
                                        [&]() { update_screen(); });
}

void Inspector::reset_refresh_timer() {
  refresh_timer_->cancel();
  refresh_timer_->reset();
}

void Inspector::intialize_screen() {
  initscr();
  curs_set(0);
  start_color();
  init_pair(static_cast<uint8_t>(Colors::kGreenOnBlack), COLOR_GREEN,
            COLOR_BLACK);
  init_pair(static_cast<uint8_t>(Colors::kRedOnBlack), COLOR_RED, COLOR_BLACK);
  init_pair(static_cast<uint8_t>(Colors::kCyanOnBlack), COLOR_CYAN,
            COLOR_BLACK);
  init_pair(static_cast<uint8_t>(Colors::kYellowOnBlack), COLOR_YELLOW,
            COLOR_BLACK);
  window_ = newpad(1000u, 1000u);
  keypad(window_, TRUE);
  nodelay(window_, TRUE);
}

void Inspector::terminate_screen() { endwin(); }

void Inspector::refresh_screen() {
  wclrtobot(window_);
  prefresh(window_, 0, 0, 0, 0, LINES - 1, COLS - 1);
  wmove(window_, 0, 0);
}

void Inspector::update_screen() {
  update_cluster_info();
  add_title("FOROS INSPECTOR");
  add_summary();
  add_newline();
  for (auto &iter : clusters_) {
    add_newline();
    add_details(iter.second);
  }
  refresh_screen();
}

void Inspector::add_summary() {
  add_subtitle("Summary");
  add_string("Name", name_column_);
  add_separater();
  add_string("Size", small_column_);
  add_separater();
  add_string("Term", small_column_);
  add_separater();
  add_string("Active", large_column_);
  add_separater();
  add_string("Leader", medium_column_);
  add_separater();
  add_string("Updated Time", large_column_);
  add_newline();
  add_string(divider_, name_column_ + large_column_ * 2 + medium_column_ +
                           small_column_ * 2 + 10);
  add_newline();
  for (auto &cluster : clusters_) {
    add_cluster_item(cluster.second);
  }
}

void Inspector::add_cluster_item(std::shared_ptr<ClusterInfo> cluster) {
  add_string(cluster->name_.c_str(), name_column_);
  add_separater();
  if (cluster->size_mismatched_) {
    add_error("mis.", small_column_);
  } else {
    add_number(cluster->size_, small_column_);
  }
  add_separater();
  add_number(cluster->term_, small_column_);
  add_separater();

  std::string active;
  for (auto &node : cluster->nodes_) {
    if (!active.empty()) {
      active += ", ";
    }
    active += std::to_string(node.second->id_);
  }
  add_string(active.c_str(), large_column_);
  add_separater();

  if (cluster->leader_exist_) {
    add_number(cluster->leader_, medium_column_);
  } else {
    add_string("N/A", medium_column_);
  }
  add_separater();

  std::stringstream updated_time;
  updated_time << std::fixed << std::setprecision(2)
               << (now() - cluster->last_updated_).seconds() << "s ago";
  add_string(updated_time.str().c_str(), large_column_);
  add_newline();
}

void Inspector::add_details(std::shared_ptr<ClusterInfo> cluster) {
  add_subtitle((std::string("Details: ") + cluster->name_).c_str());

  add_string("Node ID", medium_column_);
  add_separater();
  add_string("State", medium_column_);
  add_separater();
  add_string("Term", small_column_);
  add_separater();
  add_string("Voted For", medium_column_);
  add_separater();
  add_string("Data Size", medium_column_);
  add_separater();
  add_string("Size", small_column_);
  add_separater();
  add_string("Updated Time", large_column_);
  add_newline();
  add_string(divider_,
             large_column_ * 1 + medium_column_ * 4 + small_column_ * 2 + 12);
  add_newline();

  for (auto &node : cluster->nodes_) {
    add_node_item(node.second);
  }
}

void Inspector::add_node_item(std::shared_ptr<NodeInfo> node) {
  add_number(node->id_, medium_column_);
  add_separater();
  add_state_name(node->state_);
  add_separater();
  add_number(node->term_, small_column_);
  add_separater();
  add_number(node->voted_for_, medium_column_);
  add_separater();
  add_number(node->data_size_, medium_column_);
  add_separater();
  add_number(node->size_, small_column_);
  add_separater();
  std::stringstream updated_time;
  updated_time << std::fixed << std::setprecision(2)
               << (now() - node->last_updated_).seconds() << "s ago";
  add_string(updated_time.str().c_str(), large_column_);
  add_newline();
}

void Inspector::add_state_name(const uint8_t state) {
  switch (state) {
    case foros_msgs::msg::Inspector::STANDBY:
      add_string("Standby", medium_column_);
      break;
    case foros_msgs::msg::Inspector::FOLLOWER:
      add_string("Follower", medium_column_);
      break;
    case foros_msgs::msg::Inspector::CANDIDATE:
      add_string("Candidate", medium_column_, Colors::kYellowOnBlack);
      break;
    case foros_msgs::msg::Inspector::LEADER:
      add_bold_string("Leader", medium_column_, Colors::kGreenOnBlack);
      break;
    default:
      add_string("Unknown", medium_column_, Colors::kRedOnBlack);
      break;
  }
}

void Inspector::add_title(const std::string &str) {
  wattron(window_, A_BOLD | A_ITALIC);
  wprintw(window_, "%s\n\n", str.c_str());
  wattroff(window_, A_BOLD | A_ITALIC);
}

void Inspector::add_subtitle(const std::string &str) {
  wattron(window_, A_BOLD | A_UNDERLINE);
  wprintw(window_, "%s\n\n", str.c_str());
  wattroff(window_, A_BOLD | A_UNDERLINE);
}

void Inspector::add_string(const char *str, const uint32_t size) {
  wprintw(window_, "%*.*s", size, size, str);
}

void Inspector::add_string(const char *str, const uint32_t size, Colors color) {
  wattron(window_, COLOR_PAIR(static_cast<uint8_t>(color)));
  wprintw(window_, "%*.*s", size, size, str);
  wattroff(window_, COLOR_PAIR(static_cast<uint8_t>(color)));
}

void Inspector::add_bold_string(const char *str, const uint32_t size,
                                Colors color) {
  wattron(window_, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(color)));
  wprintw(window_, "%*.*s", size, size, str);
  wattroff(window_, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(color)));
}

void Inspector::add_number(const uint32_t num, const uint32_t size) {
  wprintw(window_, "%*u", size, num);
}
void Inspector::add_number(const uint64_t num, const uint32_t size) {
  wprintw(window_, "%*lu", size, num);
}

void Inspector::add_separater() { wprintw(window_, " |"); }

void Inspector::add_error(const char *str, const uint32_t size) {
  wattron(window_, COLOR_PAIR(static_cast<uint8_t>(Colors::kRedOnBlack)));
  wprintw(window_, "%*.*s", size, size, str);
  wattroff(window_, COLOR_PAIR(static_cast<uint8_t>(Colors::kRedOnBlack)));
}

void Inspector::add_newline() { wprintw(window_, "\n"); }

bool Inspector::is_outdated(rclcpp::Time time) {
  auto diff = now() - time;
  if (diff < std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::duration<double>(period_ * 1.5))) {
    return false;
  }
  return true;
}

std::shared_ptr<ClusterInfo> Inspector::get_cluster_info(
    const std::string &name) {
  std::shared_ptr<ClusterInfo> cluster;
  auto iter = clusters_.find(name);
  if (iter == clusters_.end() || is_outdated(iter->second->last_updated_)) {
    cluster = std::make_shared<ClusterInfo>(name);
    clusters_[name] = cluster;
  } else {
    cluster = iter->second;
  }

  return cluster;
}

std::shared_ptr<NodeInfo> Inspector::get_node_info(
    std::shared_ptr<ClusterInfo> cluster, const uint32_t id) {
  if (cluster == nullptr) {
    RCLCPP_ERROR(get_logger(), "cluster is nullptr");
    return nullptr;
  }

  std::shared_ptr<NodeInfo> node;
  auto iter = cluster->nodes_.find(id);
  if (iter == cluster->nodes_.end()) {
    node = std::make_shared<NodeInfo>(id);
    cluster->nodes_[id] = node;
  } else {
    node = iter->second;
  }

  return node;
}

void Inspector::update_cluster_info() {
  name_column_ = large_column_;
  for (auto cluster_iter = clusters_.begin();
       cluster_iter != clusters_.end();) {
    auto cluster = cluster_iter->second;
    cluster->size_mismatched_ = false;
    if (is_outdated(cluster->last_updated_)) {
      cluster_iter = clusters_.erase(cluster_iter);
    } else {
      uint32_t size = 0;
      for (auto node_iter = cluster->nodes_.begin();
           node_iter != cluster->nodes_.end();) {
        auto node = node_iter->second;
        if (is_outdated(node->last_updated_)) {
          if (cluster->leader_ == node->id_) {
            cluster->leader_exist_ = false;
          }
          node_iter = cluster->nodes_.erase(node_iter);
        } else {
          if (size != 0 && cluster->size_ != node->size_) {
            cluster->size_mismatched_ = true;
          }

          if (cluster->name_.length() > large_column_) {
            name_column_ = xlarge_column_;
          }
          size = node->size_;
          node_iter = std::next(node_iter);
        }
      }
      cluster->size_ = size;
      cluster_iter = std::next(cluster_iter);
    }
  }
}

void Inspector::inspector_message_received(
    const foros_msgs::msg::Inspector::SharedPtr msg) {
  if (is_outdated(msg->stamp)) {
    return;
  }

  auto cluster = get_cluster_info(msg->cluster_name);
  if (cluster == nullptr) {
    RCLCPP_ERROR(get_logger(), "failed to get cluster info instance");
    return;
  }

  auto node = get_node_info(cluster, msg->id);
  if (node == nullptr) {
    RCLCPP_ERROR(get_logger(), "failed to get node info instance");
    return;
  }

  if (node->last_updated_ > msg->stamp) {
    RCLCPP_ERROR(get_logger(), "outdated message received.. ignore");
    return;
  }

  node->size_ = msg->cluster_size;
  node->term_ = msg->term;
  node->state_ = msg->state;
  node->data_size_ = msg->data_size;
  node->voted_for_ = msg->voted_for;
  node->last_updated_ = msg->stamp;

  if (cluster->last_updated_ < node->last_updated_) {
    cluster->last_updated_ = node->last_updated_;
  }

  if (cluster->term_ <= node->term_) {
    cluster->term_ = node->term_;
    if (node->state_ == foros_msgs::msg::Inspector::LEADER) {
      cluster->leader_ = node->id_;
      cluster->leader_exist_ = true;
    }
  }

  reset_refresh_timer();
  update_screen();
}

double Inspector::get_period() {
  char *value = std::getenv(env_var_period_);
  if (value == nullptr) {
    return default_period_;
  }

  double period = std::atof(value);
  if (period <= 0) {
    return default_period_;
  }

  return period;
}

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit
