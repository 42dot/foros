find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_gmock REQUIRED)

ament_add_gtest(
  test_node_cluster
  test_node_cluster.cpp)
target_link_libraries(test_node_cluster ${PROJECT_NAME})

ament_add_gtest(
  test_raft_state_machine
  test_raft_state_machine.cpp)
target_link_libraries(test_raft_state_machine ${PROJECT_NAME})

ament_add_gtest(
  test_lifecycle_state_machine
  test_lifecycle_state_machine.cpp)
target_link_libraries(test_lifecycle_state_machine ${PROJECT_NAME})
