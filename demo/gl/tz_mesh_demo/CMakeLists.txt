add_demo(
	TARGET tz_mesh_demo
	SOURCE_FILES
		main.cpp
		mesh_renderer.cpp
		mesh_renderer.hpp
	SHADER_SOURCES
		demo/gl/tz_mesh_demo/shaders/mesh3d.vertex.tzsl
		demo/gl/tz_mesh_demo/shaders/mesh3d.fragment.tzsl
		demo/gl/tz_mesh_demo/shaders/preprocess.compute.tzsl
)

add_text(
	TARGET tz_mesh_demo
	INPUT_DIR ${PROJECT_SOURCE_DIR}
	OUTPUT_DIR ${PROJECT_BINARY_DIR}
	TEXT_FILES
		demo/gl/tz_mesh_demo/res/cube.gltf
)
