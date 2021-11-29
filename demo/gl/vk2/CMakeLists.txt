add_demo(
	TARGET vk2_triangle_demo
	SOURCE_FILES vk2_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/vk2/shaders/triangle_demo.vertex.tzsl
		demo/gl/vk2/shaders/triangle_demo.fragment.tzsl
)
