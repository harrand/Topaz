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
		test/gl/empty.compute.tzsl
		test/gl/six_writer.compute.tzsl
	)

add_tz_test(NAME tz_shader_test
	SOURCE_FILES tz_shader_test.cpp
	SHADER_SOURCES
		test/gl/a.vertex.tzsl
		test/gl/stdlib_test.compute.tzsl
	)
