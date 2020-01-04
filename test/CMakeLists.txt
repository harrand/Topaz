cmake_minimum_required(VERSION 3.9)

add_library(test_framework INTERFACE)
target_include_directories(test_framework INTERFACE ./)

cmake_policy(SET CMP0026 OLD)

add_subdirectory(core)
add_subdirectory(gl)
add_subdirectory(input)
add_subdirectory(memory)
add_subdirectory(render)

add_custom_target(Topaz_All_Tests)

# Register a pseudo-target which will run all tests.
macro(register_test_executable EXECUTABLE_PATH)
    set(TOPAZ_TEST_EXECUTABLES ${TOPAZ_TEST_EXECUTABLES} ${EXECUTABLE_PATH} PARENT_SCOPE)
endmacro()

function(register_test_target TEST_TARGET)
    get_property(TARGET_BINARY_LOCATION TARGET ${TEST_TARGET} PROPERTY LOCATION)
    message (STATUS "Registering target \"${TEST_TARGET}\" executable \"${TARGET_BINARY_LOCATION}\"")
    register_test_executable(${TARGET_BINARY_LOCATION})
    add_dependencies(Topaz_All_Tests ${TEST_TARGET})
    # Register the unit-test define.
    target_compile_definitions(${TEST_TARGET} PRIVATE -DTOPAZ_UNIT_TEST=1)
endfunction()

# tz::core

# tz::gl
register_test_target(tz_buffer_test)
register_test_target(tz_frame_test)
register_test_target(tz_image_test)
register_test_target(tz_object_test)
register_test_target(tz_shader_compiler_test)
register_test_target(tz_shader_preprocessor_test)
register_test_target(tz_shader_test)
register_test_target(tz_texture_test)

# tz::input
register_test_target(tz_clipboard_test)

# tz::memory
register_test_target(tz_block_test)
register_test_target(tz_pool_test)

# tz::render
register_test_target(tz_device_test)

add_custom_command(
        TARGET Topaz_All_Tests
        COMMENT "Running all tests..."
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -P ${PROJECT_SOURCE_DIR}/test/run_all_test_executables.cmake ${TOPAZ_TEST_EXECUTABLES}
)
