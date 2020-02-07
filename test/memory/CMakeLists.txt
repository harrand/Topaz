cmake_minimum_required(VERSION 3.9)

add_executable(tz_pool_test pool_test.cpp)
target_link_libraries(tz_pool_test PRIVATE topaz test_framework)

add_executable(tz_block_test block_test.cpp)
target_link_libraries(tz_block_test PRIVATE topaz test_framework)
