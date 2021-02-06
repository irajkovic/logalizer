cmake_minimum_required(VERSION 3.10)

project(log-analyzer)

add_executable(log-analyzer src/main.cpp)
set_property(TARGET log-analyzer PROPERTY CXX_STANDARD 17)

find_package(Curses REQUIRED)
target_link_libraries(log-analyzer ${CURSES_LIBRARIES} pthread)

option(ENABLE_TESTS "Build tests." OFF)
option(ENABLE_COVERAGE "Build coverage." OFF)

message("ENABLE_TESTS = ${ENABLE_TESTS}")
message("ENABLE_COVERAGE = ${ENABLE_COVERAGE}")

if (ENABLE_TESTS)
    add_subdirectory(unit_tests)
endif() 
