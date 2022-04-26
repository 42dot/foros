find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_gmock REQUIRED)

ament_add_gmock(
  test_cluster_node
  test_cluster_node.cpp)
target_link_libraries(test_cluster_node ${PROJECT_NAME})

ament_add_gtest(
  test_lifecycle_state_machine
  test_lifecycle_state_machine.cpp)
target_link_libraries(test_lifecycle_state_machine ${PROJECT_NAME})

ament_add_gmock(
  test_raft
  test_raft.cpp)
target_link_libraries(test_raft ${PROJECT_NAME})

ament_add_gmock(
  test_raft_with_inspector
  test_raft_with_inspector.cpp)
target_link_libraries(test_raft_with_inspector ${PROJECT_NAME})
