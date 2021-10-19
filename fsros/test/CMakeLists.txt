find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(
  test_node_cluster
  test_node_cluster.cpp)
target_link_libraries(test_node_cluster ${PROJECT_NAME})

ament_add_gtest(
  test_state_machine
  test_state_machine.cpp)
target_link_libraries(test_state_machine ${PROJECT_NAME})
