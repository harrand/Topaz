cmake_minimum_required(VERSION 3.9)

add_library(test_framework INTERFACE)
target_include_directories(test_framework INTERFACE ./)

cmake_policy(SET CMP0026 OLD)

add_subdirectory(core)
add_subdirectory(input)
add_subdirectory(memory)

add_custom_target(Topaz_All_Tests)

# Register a pseudo-target which will run all tests.
macro(register_test_executable EXECUTABLE_PATH)
    list(LENGTH "${TOPAZ_TEST_EXECUTABLES}" NUM_SO_FAR)
    if(NUM_SO_FAR STREQUAL 0)
        set(TOPAZ_TEST_EXECUTABLES ${EXECUTABLE_PATH} PARENT_SCOPE)
    else()
        set(TOPAZ_TEST_EXECUTABLES ${TOPAZ_TEST_EXECUTABLES} ${EXECUTABLE_PATH} PARENT_SCOPE)
    endif()
endmacro()

function(register_test_target TEST_TARGET)
    get_property(TARGET_BINARY_LOCATION TARGET ${TEST_TARGET} PROPERTY LOCATION)
    message (STATUS "Registering target \"${TEST_TARGET}\" executable \"${TARGET_BINARY_LOCATION}\"")
    register_test_executable(${TARGET_BINARY_LOCATION})
    add_dependencies(Topaz_All_Tests ${TEST_TARGET})
endfunction()

register_test_target(tz_pool_test)

add_custom_command(
        TARGET Topaz_All_Tests
        COMMENT "Running all tests..."
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -P ${PROJECT_SOURCE_DIR}/test/run_all_test_executables.cmake ${TOPAZ_TEST_EXECUTABLES}
)
