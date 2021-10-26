cmake_minimum_required(VERSION 3.8)
project(fsros_examples)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(fsros REQUIRED)

add_executable(clustering src/clustering.cpp)
ament_target_dependencies(clustering
  rclcpp
  fsros
)

install(
  TARGETS clustering EXPORT clustering
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
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
