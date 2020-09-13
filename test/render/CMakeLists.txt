cmake_minimum_required(VERSION 3.9)

add_executable(tz_device_test device_test.cpp)
target_link_libraries(tz_device_test PRIVATE topaz test_framework)

add_executable(tz_pipeline_test pipeline_test.cpp)
target_link_libraries(tz_pipeline_test PRIVATE topaz test_framework)