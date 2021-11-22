add_tz_test(NAME vk_initialise_test
	SOURCE_FILES vk_initialise_test.cpp	
	)

add_tz_test(NAME vk_descriptor_test
	SOURCE_FILES vk_descriptor_test.cpp	
	)

add_tz_test(NAME vk_graphics_pipeline_test
	SOURCE_FILES vk_graphics_pipeline_test.cpp	
	)

add_tz_test(NAME vk_logical_device_test
	SOURCE_FILES vk_logical_device_test.cpp	
	)

add_tz_test(NAME vk_physical_device_test
	SOURCE_FILES vk_physical_device_test.cpp	
	)

add_tz_test(NAME vk_pipeline_test
	SOURCE_FILES vk_pipeline_test.cpp	
	)

add_tz_test(NAME vk_render_pass_test
	SOURCE_FILES vk_render_pass_test.cpp	
	)

add_tz_test(NAME vk_shader_test
	SOURCE_FILES vk_shader_test.cpp	
	SHADER_SOURCES test/gl/vk2/shaders/empty.glsl
	)

add_tz_test(NAME vk_swapchain_test
	SOURCE_FILES vk_swapchain_test.cpp	
	)
