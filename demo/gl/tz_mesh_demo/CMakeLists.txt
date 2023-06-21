add_demo(
	TARGET tz_mesh_demo
	SOURCE_FILES
		main.cpp
		mesh_renderer.cpp
		mesh_renderer.hpp
	SHADER_SOURCES
		demo/gl/tz_mesh_demo/shaders/mesh3d.vertex.tzsl
		demo/gl/tz_mesh_demo/shaders/mesh3d.fragment.tzsl
)
