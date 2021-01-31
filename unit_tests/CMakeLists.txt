enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

# Add test cpp file
add_executable(unit_tests
    test_datamodel.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(unit_tests 
    gtest gtest_main
    pthread
)

add_test(
    NAME unit_tests 
    COMMAND unit_tests 
)
