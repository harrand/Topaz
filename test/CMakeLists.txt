function(add_tz_test)
	cmake_parse_arguments(
		ADD_TZ_TEST
		""
		"NAME"
		"SOURCE_FILES;SHADER_SOURCES;TEXT_FILES"
		${ARGN}
		)

	tz_base_add_test(
		TARGET ${ADD_TZ_TEST_NAME}
		BASE_NAME tz
		SOURCES ${ADD_TZ_TEST_SOURCE_FILES}
	)

	add_shader(
		TARGET ${ADD_TZ_TEST_NAME}
		SHADERS ${ADD_TZ_TEST_SHADER_SOURCES}
	)

	add_text(
		TARGET ${ADD_TZ_TEST_NAME}
		INPUT_DIR ${PROJECT_SOURCE_DIR}/test
		OUTPUT_DIR ${PROJECT_BINARY_DIR}/test
		TEXT_FILES ${ADD_TZ_TEST_TEXT_FILES}
	)
endfunction()

add_subdirectory(core)
add_subdirectory(gl)
add_subdirectory(lua)
