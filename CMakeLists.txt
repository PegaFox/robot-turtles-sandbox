cmake_minimum_required(VERSION 3.30)

project(robot-turtles)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(gui-lib)

add_executable(robot_turtles main.cpp)

target_link_libraries(robot_turtles gui-lib sfml-graphics sfml-window sfml-system)
