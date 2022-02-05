add_demo(
	TARGET ogl2_triangle_demo
	SOURCE_FILES ogl2_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/ogl2/shaders/triangle_demo.vertex.tzsl
		demo/gl/ogl2/shaders/triangle_demo.fragment.tzsl
)
