cmake_minimum_required(VERSION 3.9)
project(topaz_window_demo)
add_executable(topaz_window_demo
		window_demo.cpp
		window_demo.hpp
)

add_dependencies(topaz_window_demo topaz)
target_link_libraries(topaz_window_demo PRIVATE topaz)
