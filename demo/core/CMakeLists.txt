cmake_minimum_required(VERSION 3.9)

add_executable(topaz_window_demo
		window_demo.cpp
)
add_dependencies(topaz_window_demo topaz)
target_link_libraries(topaz_window_demo PRIVATE topaz)