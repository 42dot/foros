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

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "akit/failover/foros/cluster_node.hpp"
#include "akit/failover/foros/cluster_node_options.hpp"
#include "rclcpp/rclcpp.hpp"

class MyDataInterface : public akit::failover::foros::ClusterNodeDataInterface {
 public:
  explicit MyDataInterface(
      std::map<uint64_t, akit::failover::foros::Data::SharedPtr> &dataset,
      uint32_t &data_cnt, rclcpp::Logger &logger)
      : dataset_(dataset),
        data_cnt_(data_cnt),
        changed_(true),
        logger_(logger) {}

  akit::failover::foros::Data::SharedPtr on_data_get_requested(
      uint64_t id) override {
    if (id >= data_cnt_) {
      return nullptr;
    }
    dump();

    return dataset_[id];
  }

  akit::failover::foros::Data::SharedPtr on_data_get_requested() override {
    if (data_cnt_ == 0) {
      return nullptr;
    }
    dump();
    return dataset_[data_cnt_ - 1];
  }

  void on_data_rollback_requested(uint64_t id) override {
    RCLCPP_INFO(logger_, "rollback requested to %ld", id);
    data_cnt_ = id;
    changed_ = true;
    dump();
  }

  bool on_data_commit_requested(akit::failover::foros::Data::SharedPtr data) {
    if (data->id() != data_cnt_) {
      RCLCPP_ERROR(logger_, "Invalid data commit requested: %ld (latest: %d)",
                   data->id(), data_cnt_);
      return false;
    }

    dataset_[data_cnt_++] = data;
    RCLCPP_INFO(logger_, "data commited to %d", data_cnt_ - 1);
    changed_ = true;
    dump();
    return true;
  }

  void dump() {
    if (changed_ == false) {
      return;
    }
    changed_ = false;
    RCLCPP_INFO(logger_, "===== data dump =====");
    for (uint32_t i = 0; i < data_cnt_; i++) {
      RCLCPP_INFO(logger_, "%ld: %c", dataset_[i]->id(),
                  dataset_[i]->data()[0]);
    }
    RCLCPP_INFO(logger_, "=====================");
  }

 private:
  std::map<uint64_t, akit::failover::foros::Data::SharedPtr> &dataset_;
  uint32_t &data_cnt_;
  bool changed_;
  rclcpp::Logger logger_;
};

int main(int argc, char **argv) {
  const std::string kClusterName = "test_cluster";
  const std::vector<uint32_t> kClusterNodeIds = {1, 2, 3, 4};
  std::map<uint64_t, akit::failover::foros::Data::SharedPtr> dataset;

  rclcpp::Logger logger = rclcpp::get_logger(argv[0]);
  logger.set_level(rclcpp::Logger::Level::Info);

  if (argc < 3) {
    RCLCPP_ERROR(logger,
                 "Usage : %s {node ID out of 1, 2, 3, 4} {number of data}",
                 argv[0]);

    return -1;
  }

  uint32_t id = std::stoul(argv[1]);
  if (id > 4 || id == 0) {
    RCLCPP_ERROR(logger, "please use id out of 1, 2, 3, 4");
    return -1;
  }

  uint32_t data_cnt = std::stoul(argv[2]);

  unsigned char ch = 'a';
  for (uint32_t i = 0; i < data_cnt; i++) {
    auto data = akit::failover::foros::Data::make_shared(
        i, 0, std::initializer_list<uint8_t>{ch++});
    dataset[i] = data;
  }

  rclcpp::init(argc, argv);

  auto options = akit::failover::foros::ClusterNodeOptions();
  options.election_timeout_max(2000);
  options.election_timeout_min(1500);

  auto node = akit::failover::foros::ClusterNode::make_shared(
      kClusterName, id, kClusterNodeIds,
      std::make_shared<MyDataInterface>(dataset, data_cnt, logger), options);

  node->register_on_activated([&]() { RCLCPP_INFO(logger, "activated"); });
  node->register_on_deactivated([&]() { RCLCPP_INFO(logger, "deactivated"); });
  node->register_on_standby([&]() { RCLCPP_INFO(logger, "standby"); });

  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();

  return 0;
}
