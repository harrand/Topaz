cmake_minimum_required(VERSION 3.9)
project(img2cpp)
include(img2cpp.cmake)

add_executable(img2cpp
    img2cpp_main.cpp
    codegen.inl
    codegen.hpp
)
add_dependencies(img2cpp topaz)
target_link_libraries(img2cpp PRIVATE topaz)
