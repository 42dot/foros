cmake_minimum_required(VERSION 3.8)
project(foros_examples)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(foros REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

add_executable(cluster_election src/cluster_election.cpp)
ament_target_dependencies(cluster_election
  rclcpp
  foros
)

install(
  TARGETS cluster_election EXPORT cluster_election
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(cluster_service src/cluster_service.cpp)
ament_target_dependencies(cluster_service
  rclcpp
  foros
  std_srvs
)

install(
  TARGETS cluster_service EXPORT cluster_service
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(cluster_client src/cluster_client.cpp)
ament_target_dependencies(cluster_client
  rclcpp
  foros
  std_srvs
)

install(
  TARGETS cluster_client EXPORT cluster_client
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(cluster_publisher src/cluster_publisher.cpp)
ament_target_dependencies(cluster_publisher
  rclcpp
  foros
  std_msgs
)

install(
  TARGETS cluster_publisher EXPORT cluster_publisher
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(cluster_subscriber src/cluster_subscriber.cpp)
ament_target_dependencies(cluster_subscriber
  rclcpp
  foros
  std_msgs
)

install(
  TARGETS cluster_subscriber EXPORT cluster_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(cluster_log_replication_with_commit src/cluster_log_replication_with_commit.cpp)
ament_target_dependencies(cluster_log_replication_with_commit
  rclcpp
  foros
)

install(
  TARGETS cluster_log_replication_with_commit EXPORT cluster_log_replication_with_commit
  DESTINATION lib/${PROJECT_NAME}
)
add_executable(cluster_log_replication_without_commit src/cluster_log_replication_without_commit.cpp)
ament_target_dependencies(cluster_log_replication_without_commit
  rclcpp
  foros
)

install(
  TARGETS cluster_log_replication_without_commit EXPORT cluster_log_replication_without_commit
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_cmake_cpplint REQUIRED)
  ament_cpplint(FILTERS "-build/header_guard,-build/include_order")

  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()

  find_package(ament_cmake_lint_cmake REQUIRED)
  ament_lint_cmake()

  find_package(ament_cmake_xmllint REQUIRED)
  ament_xmllint()

endif()

ament_package()
