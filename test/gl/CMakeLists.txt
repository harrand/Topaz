if(${TOPAZ_RENDERAPI} MATCHES "opengl")
	add_subdirectory(ogl2)
elseif(${TOPAZ_RENDERAPI} MATCHES "vulkan")
	add_subdirectory(vk2)
endif()

add_tz_test(NAME tz_resource_test
	SOURCE_FILES tz_resource_test.cpp
	)

add_tz_test(NAME tz_renderer_test
	SOURCE_FILES tz_renderer_test.cpp
	SHADER_SOURCES
		test/gl/empty.vertex.tzsl
		test/gl/empty.fragment.tzsl
	)
