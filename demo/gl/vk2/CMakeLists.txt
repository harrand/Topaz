add_demo(
	TARGET vk2_triangle_demo_small
	SOURCE_FILES vk2_triangle_demo_small.cpp
	SHADER_SOURCES
		demo/gl/vk2/shaders/triangle_demo.vertex.tzsl
		demo/gl/vk2/shaders/triangle_demo.fragment.tzsl
)

add_demo(
	TARGET vk2_triangle_demo_medium
	SOURCE_FILES vk2_triangle_demo_medium.cpp
	SHADER_SOURCES
		demo/gl/vk2/shaders/triangle_demo_medium.vertex.tzsl
		demo/gl/vk2/shaders/triangle_demo_medium.fragment.tzsl
)

add_demo(
	TARGET vk2_triangle_demo_large
	SOURCE_FILES vk2_triangle_demo_large.cpp
	SHADER_SOURCES
		demo/gl/vk2/shaders/triangle_demo_large.vertex.tzsl
		demo/gl/vk2/shaders/triangle_demo_large.fragment.tzsl
)
