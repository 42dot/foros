find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(
  test_cluster_node
  test_cluster_node.cpp)
target_link_libraries(test_cluster_node ${PROJECT_NAME})

ament_add_gtest(
  test_raft_state_machine
  test_raft_state_machine.cpp)
target_link_libraries(test_raft_state_machine ${PROJECT_NAME})

ament_add_gtest(
  test_lifecycle_state_machine
  test_lifecycle_state_machine.cpp)
target_link_libraries(test_lifecycle_state_machine ${PROJECT_NAME})

ament_add_gtest(
  test_publisher
  test_publisher.cpp)
target_link_libraries(test_publisher ${PROJECT_NAME})
ament_target_dependencies(test_publisher "std_msgs")

