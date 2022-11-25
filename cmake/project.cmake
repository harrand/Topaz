include(CMakeParseArguments)

function(tz_add_library)
	cmake_parse_arguments(
		TZ_ADD_LIBRARY
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)

	hdk_base_add_library(
		TARGET ${TZ_ADD_LIBRARY_TARGET}
		BASE topaz
		SOURCES ${TZ_ADD_LIBRARY_SOURCES}
	)
endfunction()

function(tz_add_executable)
	cmake_parse_arguments(
		TZ_ADD_EXECUTABLE
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)

	hdk_base_add_executable(
		TARGET ${TZ_ADD_EXECUTABLE_TARGET}
		BASE topaz
		SOURCES ${TZ_ADD_EXECUTABLE_SOURCES}
	)
endfunction()
