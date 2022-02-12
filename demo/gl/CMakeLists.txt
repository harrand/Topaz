if(${TOPAZ_VULKAN})
	add_subdirectory(vk2)
elseif(${TOPAZ_OGL})
	add_subdirectory(ogl2)
endif()

add_demo(
	TARGET tz_triangle_demo
	SOURCE_FILES tz_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_triangle_demo.vertex.tzsl
		demo/gl/shaders/tz_triangle_demo.fragment.tzsl
)

add_demo(
	TARGET tz_dynamic_triangle_demo
	SOURCE_FILES tz_dynamic_triangle_demo.cpp
	SHADER_SOURCES
		demo/gl/shaders/tz_dynamic_triangle_demo.vertex.tzsl
		demo/gl/shaders/tz_dynamic_triangle_demo.fragment.tzsl
)