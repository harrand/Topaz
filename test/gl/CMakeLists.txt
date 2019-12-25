cmake_minimum_required(VERSION 3.9)
add_executable(tz_buffer_test buffer_test.cpp)
target_link_libraries(tz_buffer_test PRIVATE topaz test_framework)
