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
#include <sstream>
#include <string>

#include "terminal_utils.hpp"

namespace akit {
namespace failover {
namespace foros_inspector {

const std::string Inspector::kNodeName = "foros_inspector";

Inspector::Inspector() : rclcpp::Node(kNodeName) {
  subscriber_ = create_subscription<foros_msgs::msg::Inspector>(
      foros_msgs::msg::Inspector::TOPIC_NAME, 10,
      std::bind(&Inspector::inspector_message_received, this,
                std::placeholders::_1));
  intialize_screen();
  initialize_refresh_timer();
}

Inspector::~Inspector() { terminate_screen(); }

void Inspector::initialize_refresh_timer() {
  refresh_timer_ =
      rclcpp::create_timer(this, rclcpp::Clock::make_shared(),
                           std::chrono::seconds(1), [&]() { update_screen(); });
}

void Inspector::reset_refresh_timer() {
  refresh_timer_->cancel();
  refresh_timer_->reset();
}

void Inspector::intialize_screen() {
  initscr();
  curs_set(0);
  start_color();
  init_pair(static_cast<uint8_t>(Colors::kRedOnBlack), COLOR_GREEN,
            COLOR_BLACK);
  init_pair(static_cast<uint8_t>(Colors::kWhiteOnRed), COLOR_WHITE, COLOR_RED);
  pad = newpad(1000u, 1000u);
  keypad(pad, TRUE);
  nodelay(pad, TRUE);
}

void Inspector::terminate_screen() { endwin(); }

void Inspector::refresh_screen() {
  wclrtobot(pad);
  prefresh(pad, 0, 0, 0, 0, LINES - 1, COLS - 1);
  wmove(pad, 0, 0);
}

void Inspector::update_screen() {
  remove_outdated_cluster_info();
  add_title("FOROS INSPECTOR");
  add_summary();
  add_padding();
  for (auto iter : clusters_) {
    add_padding();
    add_details(iter.second);
  }
  refresh_screen();
}

void Inspector::add_summary() {
  add_subtitle("Summary");

  wprintw(pad, "%*s |", large_column_, "Name");
  wprintw(pad, "%*s |", medium_column_, "Size");
  wprintw(pad, "%*s |", medium_column_, "Term");
  wprintw(pad, "%*s |", large_column_, "Active");
  wprintw(pad, "%*s |", medium_column_, "Leader");
  wprintw(pad, "%*s\n", large_column_, "Updated Time");

  const int32_t total_column = large_column_ * 3 + medium_column_ * 3 + 10;
  wprintw(pad, "%*.*s\n", total_column, total_column, divider_);

  for (auto cluster : clusters_) {
    wprintw(pad, "%*.*s |", large_column_, large_column_,
            cluster.second->name_.c_str());
    wprintw(pad, "%*d |", medium_column_, cluster.second->size_);
    wprintw(pad, "%*lu |", medium_column_, cluster.second->term_);

    std::string active;
    for (auto node : cluster.second->nodes_) {
      if (!active.empty()) {
        active += ", ";
      }
      active += std::to_string(node.second->id_);
    }

    wprintw(pad, "%*s |", large_column_, active.c_str());
    if (cluster.second->leader_exist_) {
      wprintw(pad, "%*d |", medium_column_, cluster.second->leader_);
    } else {
      wprintw(pad, "%*s |", medium_column_, "N/A");
    }

    std::stringstream updated_time;
    updated_time << std::fixed << std::setprecision(2)
                 << (now() - cluster.second->last_updated_).seconds()
                 << "s ago";

    wprintw(pad, "%*s\n", large_column_, updated_time.str().c_str());
  }
}

void Inspector::add_details(std::shared_ptr<ClusterInfo> cluster) {
  add_subtitle((std::string("Details: ") + cluster->name_).c_str());

  wprintw(pad, "%*s |", medium_column_, "Node ID");
  wprintw(pad, "%*s |", large_column_, "State");
  wprintw(pad, "%*s |", medium_column_, "Term");
  wprintw(pad, "%*s |", medium_column_, "Voted For");
  wprintw(pad, "%*s |", medium_column_, "Data Size");
  wprintw(pad, "%*s\n", large_column_, "Updated Time");

  const int32_t total_column = large_column_ * 2 + medium_column_ * 4 + 10;
  wprintw(pad, "%*.*s\n", total_column, total_column, divider_);

  for (auto node : cluster->nodes_) {
    wprintw(pad, "%*u |", medium_column_, node.second->id_);
    wprintw(pad, "%*s |", large_column_, get_state_name(node.second->state_));
    wprintw(pad, "%*lu |", medium_column_, node.second->term_);
    wprintw(pad, "%*u |", medium_column_, node.second->voted_for_);
    wprintw(pad, "%*lu |", medium_column_, node.second->data_size_);

    std::stringstream updated_time;
    updated_time << std::fixed << std::setprecision(2)
                 << (now() - node.second->last_updated_).seconds() << "s ago";

    wprintw(pad, "%*s\n", large_column_, updated_time.str().c_str());
  }
}

const char *Inspector::get_state_name(const uint8_t state) {
  switch (state) {
    case foros_msgs::msg::Inspector::STANDBY:
      return "Standby";
    case foros_msgs::msg::Inspector::FOLLOWER:
      return "Follower";
    case foros_msgs::msg::Inspector::CANDIDATE:
      return "Candidate";
    case foros_msgs::msg::Inspector::LEADER:
      return "Leader";
  }
  return "Unknown";
}

void Inspector::add_title(const std::string &str) {
  wattron(pad, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(Colors::kRedOnBlack)));
  wprintw(pad, "%s\n\n", str.c_str());
  wattroff(pad, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(Colors::kRedOnBlack)));
}

void Inspector::add_subtitle(const std::string &str) {
  wattron(pad, A_BOLD | A_UNDERLINE);
  wprintw(pad, "%s\n\n", str.c_str());
  wattroff(pad, A_BOLD | A_UNDERLINE);
}

void Inspector::add_padding() { wprintw(pad, "\n"); }

bool Inspector::is_outdated(rclcpp::Time time) {
  auto diff = now() - time;
  if (diff < std::chrono::milliseconds(1500)) {
    return false;
  }
  return true;
}

std::shared_ptr<ClusterInfo> Inspector::get_cluster_info(
    const std::string &name, const uint32_t size) {
  std::shared_ptr<ClusterInfo> cluster;
  auto iter = clusters_.find(name);
  if (iter == clusters_.end() || is_outdated(iter->second->last_updated_)) {
    cluster = std::make_shared<ClusterInfo>(name, size);
    clusters_[name] = cluster;
  } else {
    cluster = iter->second;
    if (cluster->size_ != size) {
      RCLCPP_ERROR(get_logger(), "cluster %s: size mismatched: %d %d",
                   name.c_str(), cluster->size_, size);
      cluster->size_ = size;
    }
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

void Inspector::remove_outdated_cluster_info() {
  auto n = now();
  for (auto cluster_iter = clusters_.begin();
       cluster_iter != clusters_.end();) {
    auto cluster = cluster_iter->second;
    if (is_outdated(cluster->last_updated_)) {
      cluster_iter = clusters_.erase(cluster_iter);
    } else {
      for (auto node_iter = cluster->nodes_.begin();
           node_iter != cluster->nodes_.end();) {
        if (is_outdated(node_iter->second->last_updated_)) {
          if (cluster->leader_ == node_iter->second->id_) {
            cluster->leader_exist_ = false;
          }
          node_iter = cluster->nodes_.erase(node_iter);
        } else {
          node_iter = std::next(node_iter);
        }
      }
      cluster_iter = std::next(cluster_iter);
    }
  }
}

void Inspector::inspector_message_received(
    const foros_msgs::msg::Inspector::SharedPtr msg) {
  if (is_outdated(msg->stamp)) {
    return;
  }

  auto cluster = get_cluster_info(msg->cluster_name, msg->cluster_size);
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

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit
