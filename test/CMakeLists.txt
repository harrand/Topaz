function(add_tz_test)
	cmake_parse_arguments(
		ADD_TZ_TEST
		""
		"NAME"
		"SOURCE_FILES;SHADER_SOURCES"
		${ARGN}
		)

	hdk_base_add_test(
		TARGET ${ADD_TZ_TEST_NAME}
		BASE_NAME tz
		SOURCES ${ADD_TZ_TEST_SOURCE_FILES}
	)

	add_shader(
		TARGET ${ADD_TZ_TEST_NAME}
		SHADERS ${ADD_TZ_TEST_SHADER_SOURCES}
	)
endfunction()

hdk_base_add_test_harness(BASE_NAME tz)

add_subdirectory(core)
add_subdirectory(gl)
