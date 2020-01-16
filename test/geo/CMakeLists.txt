cmake_minimum_required(VERSION 3.9)

add_executable(tz_vector_test vector_test.cpp)
target_link_libraries(tz_vector_test PRIVATE topaz test_framework)

add_executable(tz_matrix_test matrix_test.cpp)
target_link_libraries(tz_matrix_test PRIVATE topaz test_framework)