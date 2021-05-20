function(add_tz_test)
    cmake_parse_arguments(
        ADD_TZ_TEST
        ""
        "NAME"
        "SOURCE_FILES"
        ${ARGN}
        )

    add_executable(${ADD_TZ_TEST_NAME}
        ${ADD_TZ_TEST_SOURCE_FILES}
    )
    target_include_directories(${ADD_TZ_TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
    target_link_libraries(${ADD_TZ_TEST_NAME} PRIVATE topaz tz_test_harness)
    add_test(NAME ${ADD_TZ_TEST_NAME}
        COMMAND ${ADD_TZ_TEST_NAME})
    add_dependencies(tztest ${ADD_TZ_TEST_NAME})
endfunction()

add_custom_target(tztest COMMAND "${CMAKE_CTEST_COMMAND}" "--output-on-failure"
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_library(tz_test_harness INTERFACE)
target_include_directories(tz_test_harness INTERFACE ./)
add_subdirectory(core)