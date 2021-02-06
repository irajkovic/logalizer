message("Bulding tests..")

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

add_custom_target(run-tests
    COMMAND ${CMAKE_BINARY_DIR}/unit_tests/unit_tests
    DEPENS unit_tests)

if (ENABLE_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    message("Bulding code coverage..")
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
    add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
    target_link_libraries(unit_tests ${GCC_COVERAGE_LINK_FLAGS})

    add_custom_target(coverage 
        DEPENDS run-tests
        COMMAND lcov -c -i --directory ${CMAKE_SOURCE_DIR} --no-external -o base-coverage.info
        COMMAND lcov -c --directory ${CMAKE_SOURCE_DIR} --no-external --output-file run-coverage.info
        COMMAND lcov -a base-coverage.info -a run-coverage.info -o coverage.info
        COMMAND genhtml coverage.info --output-directory coverage-results
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
