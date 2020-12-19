cmake_minimum_required(VERSION 3.10)

project(log-analyzer)

add_executable(log-analyzer main.cpp)

find_package(Curses REQUIRED)
target_link_libraries(log-analyzer ${CURSES_LIBRARIES} pthread)
