cmake_minimum_required(VERSION 3.10)

project(log-analyzer)

add_executable(log-analyzer main.cpp)
set_property(TARGET log-analyzer PROPERTY CXX_STANDARD 17)

find_package(Curses REQUIRED)
target_link_libraries(log-analyzer ${CURSES_LIBRARIES} pthread)
