cmake_minimum_required(VERSION 3.9)
project(topaz_graphics_demo)

add_executable(topaz_graphics_demo
		graphics_demo.cpp
		graphics_demo.hpp
)

add_dependencies(topaz_graphics_demo topaz)
target_link_libraries(topaz_graphics_demo PRIVATE topaz)

add_executable(topaz_ssbo_tzglp_demo
		ssbo_tzglp_demo.cpp
)

add_dependencies(topaz_ssbo_tzglp_demo topaz)
target_link_libraries(topaz_ssbo_tzglp_demo PRIVATE topaz)