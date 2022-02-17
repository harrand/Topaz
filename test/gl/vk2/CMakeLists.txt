add_tz_test(NAME vk_initialise_test
	SOURCE_FILES vk_initialise_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_buffer_test
	SOURCE_FILES vk_buffer_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_command_test
	SOURCE_FILES vk_command_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_descriptor_test
	SOURCE_FILES vk_descriptor_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_fence_test
	SOURCE_FILES vk_fence_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_framebuffer_test
	SOURCE_FILES vk_framebuffer_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_graphics_pipeline_test
	SOURCE_FILES vk_graphics_pipeline_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_image_test
	SOURCE_FILES vk_image_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_logical_device_test
	SOURCE_FILES vk_logical_device_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_physical_device_test
	SOURCE_FILES vk_physical_device_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_pipeline_test
	SOURCE_FILES vk_pipeline_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_render_pass_test
	SOURCE_FILES vk_render_pass_test.cpp
	GRAPHICAL
	)

add_tz_test(NAME vk_shader_test
	SOURCE_FILES vk_shader_test.cpp	
	SHADER_SOURCES test/gl/vk2/empty.glsl
	GRAPHICAL
	)

add_tz_test(NAME vk_swapchain_test
	SOURCE_FILES vk_swapchain_test.cpp
	GRAPHICAL
	)
