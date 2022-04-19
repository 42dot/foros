cmake_minimum_required(VERSION 3.8)
project(foros_inspector)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(foros_msgs REQUIRED)
set(CURSES_USE_NCURSES TRUE)
find_package(Curses REQUIRED)

include_directories(src)

set(${PROJECT_NAME}_SRCS
  src/main.cpp
  src/inspector.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})
ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  foros_msgs
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CURSES_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  ncurses
)

install(
  TARGETS ${PROJECT_NAME} ${PROJECT_NAME}
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
