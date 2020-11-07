cmake_minimum_required(VERSION 3.9)
project(topaz_render_demo)

add_executable(topaz_render_demo
		render_demo.cpp
)
add_dependencies(topaz_render_demo topaz)
target_link_libraries(topaz_render_demo PRIVATE topaz)

add_executable(topaz_postprocess_demo
		postprocess_demo.cpp
)
add_dependencies(topaz_postprocess_demo topaz)
target_link_libraries(topaz_postprocess_demo PRIVATE topaz)

add_executable(topaz_lighting_demo
		lighting_demo.cpp
)
add_dependencies(topaz_lighting_demo topaz)
target_link_libraries(topaz_lighting_demo PRIVATE topaz)