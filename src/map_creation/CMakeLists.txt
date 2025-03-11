cmake_minimum_required(VERSION 3.0.2)
project(map_creation)

# Find required packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  lanelet2_core
  lanelet2_io
)

find_package(Eigen3 REQUIRED)
include_directories(
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
)

catkin_package(
  CATKIN_DEPENDS roscpp lanelet2_core lanelet2_io visualization_msgs
)

add_executable(map_main src/main.cpp)

target_link_libraries(map_main
  ${catkin_LIBRARIES}
)

install(DIRECTORY launch config
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
  FILES_MATCHING PATTERN "*.yaml" PATTERN "*.launch"
)

