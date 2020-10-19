cmake_minimum_required(VERSION 3.9)
project(tzglp)

add_executable(tzglp
    tzglp.cpp
    tzglp.hpp
    tzglp_main.cpp
)
add_dependencies(tzglp topaz)
target_link_libraries(tzglp PRIVATE topaz)
