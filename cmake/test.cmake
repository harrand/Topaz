include(CMakeParseArguments)

function(tz_base_add_test_harness)
	cmake_parse_arguments(
		TZ_BASE_ADD_TEST_HARNESS
		""
		"BASE_NAME"
		""
		${ARGN}
	)

	cmake_language(EVAL CODE
	"
	add_custom_target(${TZ_BASE_ADD_TEST_HARNESS_BASE_NAME}test_build)
	add_custom_target(${TZ_BASE_ADD_TEST_HARNESS_BASE_NAME}test
		COMMAND \"${CMAKE_CTEST_COMMAND}\" --output-on-failure --schedule-random -L \"${TZ_BASE_ADD_TEST_HARNESS_BASE_NAME}\"
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		)
	")
	
endfunction()

function(tz_base_add_test)
	cmake_parse_arguments(
		TZ_BASE_ADD_TEST
		""
		"TARGET;BASE_NAME"
		"SOURCES"
		${ARGN}
	)

	cmake_language(EVAL CODE
	"
		${TZ_BASE_ADD_TEST_BASE_NAME}_add_executable(
			TARGET ${TZ_BASE_ADD_TEST_TARGET}
			SOURCES ${TZ_BASE_ADD_TEST_SOURCES}
		)

		add_test(
			NAME ${TZ_BASE_ADD_TEST_TARGET}
			COMMAND ${TZ_BASE_ADD_TEST_TARGET}
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		)
		set_tests_properties(${TZ_BASE_ADD_TEST_TARGET} PROPERTIES LABELS \"${TZ_BASE_ADD_TEST_BASE_NAME}\")

		add_dependencies(${TZ_BASE_ADD_TEST_BASE_NAME}test_build ${TZ_BASE_ADD_TEST_TARGET})
		add_dependencies(${TZ_BASE_ADD_TEST_BASE_NAME}test ${TZ_BASE_ADD_TEST_TARGET})
		message(STATUS \"${TZ_BASE_ADD_TEST_BASE_NAME} - Detected test ${TZ_BASE_ADD_TEST_TARGET}\")
	")
	
endfunction()

function(tz_add_test)
	cmake_parse_arguments(
		TZ_ADD_TEST
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)

	tz_base_add_test(
		TARGET ${TZ_ADD_TEST_TARGET}
		BASE_NAME tz
		SOURCES ${TZ_ADD_TEST_SOURCES}
	)
	target_compile_definitions(${TZ_ADD_TEST_TARGET} PRIVATE "-DTRACY_NO_EXIT=1")
endfunction()
tz_base_add_test_harness(BASE_NAME tz)
