cmake_minimum_required(VERSION 3.9)

add_executable(topaz_pc_input_demo
		pc_input_demo.cpp
)
add_dependencies(topaz_pc_input_demo topaz)
target_link_libraries(topaz_pc_input_demo PRIVATE topaz)