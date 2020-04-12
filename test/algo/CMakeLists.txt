cmake_minimum_required(VERSION 3.9)

add_executable(tz_container_test container_test.cpp)
target_link_libraries(tz_container_test PRIVATE topaz test_framework)

add_executable(tz_math_test math_test.cpp)
target_link_libraries(tz_math_test PRIVATE topaz test_framework)
