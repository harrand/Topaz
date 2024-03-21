function(topaz_add_test)
	cmake_parse_arguments(
		TOPAZ_ADD_TEST
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)	

	add_executable(${TOPAZ_ADD_TEST_TARGET}
		${TOPAZ_ADD_TEST_SOURCES}
	)
	target_link_libraries(${TOPAZ_ADD_TEST_TARGET} PRIVATE topaz)
	add_test(${TOPAZ_ADD_TEST_TARGET}_build
		"${CMAKE_COMMAND}"
		--build "${CMAKE_BINARY_DIR}"
		--config "$<CONFIG>"
		--target ${TOPAZ_ADD_TEST_TARGET}
	)
	set_tests_properties(${TOPAZ_ADD_TEST_TARGET}_build PROPERTIES FIXTURES_SETUP ${TOPAZ_ADD_TEST_TARGET}_fixture)
	add_test(${TOPAZ_ADD_TEST_TARGET} ${TOPAZ_ADD_TEST_TARGET})
	set_tests_properties(${TOPAZ_ADD_TEST_TARGET} PROPERTIES FIXTURES_REQUIRED ${TOPAZ_ADD_TEST_TARGET}_fixture)
endfunction()

if(BUILD_TESTING)
	add_subdirectory(tz)
endif()