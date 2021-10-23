find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(
  test_node_cluster
  test_node_cluster.cpp)
target_link_libraries(test_node_cluster ${PROJECT_NAME})

ament_add_gtest(
  test_raft_state_machine
  test_raft_state_machine.cpp)
target_link_libraries(test_raft_state_machine ${PROJECT_NAME})

ament_add_gtest(
  test_lifecycle
  test_lifecycle.cpp)
target_link_libraries(test_lifecycle ${PROJECT_NAME})
