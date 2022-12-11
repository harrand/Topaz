if(${TOPAZ_RENDER_API} MATCHES "OpenGL")
	add_subdirectory(ogl2)
endif()

add_tz_test(NAME tz_resource_test
	SOURCE_FILES tz_resource_test.cpp
	)

add_tz_test(NAME tz_renderer_test
	SOURCE_FILES tz_renderer_test.cpp
	SHADER_SOURCES
		test/gl/shaders/empty.vertex.tzsl
		test/gl/shaders/empty.fragment.tzsl
		test/gl/shaders/empty.compute.tzsl
		test/gl/shaders/six_writer.compute.tzsl
	)

add_tz_test(NAME tz_shader_test
	SOURCE_FILES tz_shader_test.cpp
	SHADER_SOURCES
		test/gl/shaders/shader_test.compute.tzsl
	)
