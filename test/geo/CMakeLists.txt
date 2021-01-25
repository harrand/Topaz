cmake_minimum_required(VERSION 3.9)

add_executable(tz_aabb_test aabb_test.cpp)
target_link_libraries(tz_aabb_test PRIVATE topaz test_framework)

add_executable(tz_sphere_test sphere_test.cpp)
target_link_libraries(tz_sphere_test PRIVATE topaz test_framework)