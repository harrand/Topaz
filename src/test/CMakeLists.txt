cmake_minimum_required(VERSION 3.9)

add_library(unit_testing_utility lib/test_failure_exception.cpp)
set_target_properties(unit_testing_utility
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        )

add_custom_target(topaz_all_tests COMMENT Building ALL unit tests)
# Globbing is evil, but without monsters there can be no heroes
file(GLOB UNIT_TEST_SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/src/test/ *.cpp)
foreach(unit_test_source ${UNIT_TEST_SOURCES})
    # Cut off the '.cpp', so the target name for test_vector.cpp becomes test_vector.
    string(REPLACE ".cpp" "" unit_test_name ${unit_test_source})
    message("found module: " ${unit_test_name})
    add_executable(${unit_test_name} ${unit_test_source})
    set_target_properties(${unit_test_name}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
            )
    target_include_directories(${unit_test_name} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${unit_test_name} topaz unit_testing_utility)
    add_dependencies(topaz_all_tests ${unit_test_name})
endforeach()
set_target_properties(topaz_all_tests
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        )