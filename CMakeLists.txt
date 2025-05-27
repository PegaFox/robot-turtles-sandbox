cmake_minimum_required(VERSION 3.30)

project(robot-turtles)

set(CMAKE_CXX_STANDARD 20)

link_libraries(sfml-graphics sfml-window sfml-system)

add_executable(robot_turtles main.cpp)