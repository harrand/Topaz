cmake_minimum_required(VERSION 3.9)
project(topaz_physics_demo)

add_executable(topaz_physics_demo
		physics_demo.cpp
)

add_dependencies(topaz_physics_demo topaz)
target_link_libraries(topaz_physics_demo PRIVATE topaz)