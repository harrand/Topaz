if(${TOPAZ_VULKAN})
	add_subdirectory(vk2)
elseif(${TOPAZ_OGL})
	add_subdirectory(ogl2)
endif()

add_demo(
	TARGET tz_terrain_demo
	SOURCE_FILES tz_terrain_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_terrain_demo.vertex.tzsl
		demo/gl/shaders/tz_terrain_demo.tesscon.tzsl
		demo/gl/shaders/tz_terrain_demo.tesseval.tzsl
		demo/gl/shaders/tz_terrain_demo.fragment.tzsl
		demo/gl/shaders/tz_triangle_demo.vertex.tzsl
		demo/gl/shaders/tz_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_triangle_demo
	SOURCE_FILES tz_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_triangle_demo.vertex.tzsl
		demo/gl/shaders/tz_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_blur_triangle_demo
	SOURCE_FILES tz_blur_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_triangle_demo.vertex.tzsl
		demo/gl/shaders/tz_triangle_demo.fragment.tzsl
		demo/gl/shaders/blur.vertex.tzsl
		demo/gl/shaders/blur.fragment.tzsl
)

add_demo(
	TARGET tz_dynamic_triangle_demo
	SOURCE_FILES tz_dynamic_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_dynamic_triangle_demo.vertex.tzsl
		demo/gl/shaders/tz_dynamic_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_compute_demo
	SOURCE_FILES tz_compute_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_compute_demo.compute.tzsl
		demo/gl/shaders/tz_compute_demo_render.vertex.tzsl
		demo/gl/shaders/tz_compute_demo_render.fragment.tzsl
)
