function(add_tz_test)
	cmake_parse_arguments(
		ADD_TZ_TEST
		""
		"NAME"
		"SOURCE_FILES;SHADER_SOURCES"
		${ARGN}
		)

	add_executable(${ADD_TZ_TEST_NAME}
		${ADD_TZ_TEST_SOURCE_FILES}
	)

	set_target_properties(${ADD_TZ_TEST_NAME} PROPERTIES
		CXX_STANDARD ${TZ_CXX_STANDARD}
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF)

	target_include_directories(${ADD_TZ_TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
	target_link_libraries(${ADD_TZ_TEST_NAME} PRIVATE topaz tz_test_harness)
	add_shader(
		TARGET ${ADD_TZ_TEST_NAME}
		SHADERS ${ADD_TZ_TEST_SHADER_SOURCES}
	)
	add_test(NAME ${ADD_TZ_TEST_NAME}
		COMMAND ${ADD_TZ_TEST_NAME}
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
	add_dependencies(tztest_build ${ADD_TZ_TEST_NAME})
	add_dependencies(tztest ${ADD_TZ_TEST_NAME})
	message(STATUS "Detected test ${ADD_TZ_TEST_NAME}")
endfunction()

add_custom_target(tztest_build)
add_custom_target(tztest COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure --schedule-random -C Debug
						WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_library(tz_test_harness INTERFACE)
target_include_directories(tz_test_harness INTERFACE ./)

add_subdirectory(core)
add_subdirectory(gl)
