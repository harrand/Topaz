topaz_add_demo(
	TARGET tz_triangle_demo
	SOURCES
		${PROJECT_SOURCE_DIR}/demo/tz/tz_triangle_demo.cpp
	SHADERS
		demo/tz/shaders/triangle.vertex.tzsl
		demo/tz/shaders/triangle.fragment.tzsl
)