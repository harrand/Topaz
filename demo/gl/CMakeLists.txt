add_subdirectory(vk2)

add_demo(
	TARGET tz_triangle_demo
	SOURCE_FILES tz_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/tz_triangle_demo.vertex.tzsl
		demo/gl/tz_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_compute_demo
	SOURCE_FILES tz_compute_demo.cpp
	SHADER_SOURCES
		demo/gl/compute_demo.compute.tzsl
)

add_demo(
	TARGET tz_dynamic_triangle_demo
	SOURCE_FILES tz_dynamic_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/dynamic_triangle_demo.vertex.tzsl
		demo/gl/dynamic_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_invert_triangle_demo
	SOURCE_FILES tz_invert_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/triangle_demo.vertex.tzsl
		demo/gl/triangle_demo.fragment.tzsl
		demo/gl/invert_triangle_demo.vertex.tzsl
		demo/gl/invert_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_multi_draw_demo
	SOURCE_FILES tz_multi_draw_demo.cpp
	SHADER_SOURCES
		demo/gl/dynamic_triangle_demo.vertex.tzsl
		demo/gl/dynamic_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET vkfront_temp_demo
	SOURCE_FILES vkfront_temp_demo.cpp
	SHADER_SOURCES
		demo/gl/vkfront_temp_demo.vertex.tzsl
		demo/gl/vkfront_temp_demo.fragment.tzsl
)
