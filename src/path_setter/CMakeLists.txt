cmake_minimum_required(VERSION 3.0.2)
project(path_setter)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  lanelet2_core
  lanelet2_io
  lanelet2_routing
  nav_msgs
)

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp lanelet2_core lanelet2_io lanelet2_routing nav_msgs
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(main src/main.cpp)

target_link_libraries(main
  ${catkin_LIBRARIES}
)
install(DIRECTORY launch config
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
  FILES_MATCHING PATTERN "*.yaml" PATTERN "*.launch"
)

