function(topaz_add_test)
	cmake_parse_arguments(
		TOPAZ_ADD_TEST
		""
		"TARGET"
		"SOURCES;SHADERS"
		${ARGN}
	)	

	topaz_add_executable(
		TARGET ${TOPAZ_ADD_TEST_TARGET}
		SOURCES
			${TOPAZ_ADD_TEST_SOURCES}
		SHADERS
			${TOPAZ_ADD_TEST_SHADERS}
	)
	# All tests link against topaz.
	target_link_libraries(${TOPAZ_ADD_TEST_TARGET} PRIVATE topaz)
	# Note: CTest only runs test executables, doesn't build them. This is of course unfathomably stupid - You have to use "fixtures" (see: https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/)
	# I'm just parroting what it says to do here.
	# Essentially:
	# 1. Add a test to simply run cmake --build to build the target.
	# 2. Set a "fixture" property on (1).
	# 3. Add a test to simply run the executable, this is normal CMake stuff.
	# 4. Set a fixture requirement on (3) against (2), causing it to essentially depend on building itself.
	# (1)
	add_test(${TOPAZ_ADD_TEST_TARGET}_build
		"${CMAKE_COMMAND}"
		--build "${CMAKE_BINARY_DIR}"
		--config "$<CONFIG>"
		--target ${TOPAZ_ADD_TEST_TARGET}
	)
	# (2)
	set_tests_properties(${TOPAZ_ADD_TEST_TARGET}_build PROPERTIES FIXTURES_SETUP ${TOPAZ_ADD_TEST_TARGET}_fixture)
	# (3)
	add_test(${TOPAZ_ADD_TEST_TARGET} ${TOPAZ_ADD_TEST_TARGET})
	# (4)
	set_tests_properties(${TOPAZ_ADD_TEST_TARGET} PROPERTIES FIXTURES_REQUIRED ${TOPAZ_ADD_TEST_TARGET}_fixture)
endfunction()

# When top-level topaz CMakeLists includes CTest, this `BUILD_TESTING` option is created which defaults to ON. I guess the idea is that this is a cache variable you can turn off if you really want to.
# Seems a bit silly to me, but we'll respect it anyway.
if(BUILD_TESTING)
	add_subdirectory(tz)
endif()