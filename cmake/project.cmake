include(CMakeParseArguments)

function(tz_configure_common)
	cmake_parse_arguments(
		TZ_CONFIGURE_COMMON
		""
		"TARGET"
		""
		${ARGN}
	)

	set_target_properties(${TZ_CONFIGURE_COMMON_TARGET} PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)
endfunction()

function(tz_configure_debug)
	cmake_parse_arguments(
		TZ_CONFIGURE_DEBUG
		""
		"TARGET"
		""
		${ARGN}
	)

	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_compile_options(${TZ_CONFIGURE_DEBUG_TARGET} PUBLIC /Zi)
	else()
		target_compile_options(${TZ_CONFIGURE_DEBUG_TARGET} PUBLIC -g -Og)
	endif()

endfunction()

function(tz_configure_release)
	cmake_parse_arguments(
		TZ_CONFIGURE_RELEASE
		""
		"TARGET"
		""
		${ARGN}
	)

	target_compile_options(${TZ_CONFIGURE_RELEASE_TARGET}
		PRIVATE
		-O3
	)

endfunction()

function(tz_configure_profile)
	cmake_parse_arguments(
		TZ_CONFIGURE_PROFILE
		""
		"TARGET"
		""
		${ARGN}
	)

	# Tracy Specific:
	target_link_libraries(${TZ_CONFIGURE_PROFILE_TARGET} PUBLIC TracyClient)
	if(WIN32)
		target_link_libraries(${TZ_CONFIGURE_PROFILE_TARGET} PUBLIC ws2_32 dbghelp)
	endif()
endfunction()


function(tz_base_add_library)
	cmake_parse_arguments(
		TZ_BASE_ADD_LIBRARY
		""
		"TARGET;BASE"
		"SOURCES"
		${ARGN}
	)
	
	cmake_language(EVAL CODE
	"
		add_library(${TZ_BASE_ADD_LIBRARY_TARGET} STATIC
			${TZ_BASE_ADD_LIBRARY_SOURCES})

		tz_configure_common(
			TARGET ${TZ_BASE_ADD_LIBRARY_TARGET}
		)

	target_link_libraries(${TZ_BASE_ADD_LIBRARY_TARGET} PUBLIC ${TZ_BASE_ADD_LIBRARY_BASE})
	")
endfunction()

function(tz_base_add_executable)
	cmake_parse_arguments(
		TZ_BASE_ADD_EXECUTABLE
		""
		"TARGET;BASE"
		"SOURCES"
		${ARGN}
	)
	
	cmake_language(EVAL CODE
	"
		add_executable(${TZ_BASE_ADD_EXECUTABLE_TARGET}
			${TZ_BASE_ADD_EXECUTABLE_SOURCES}
		)

		tz_configure_common(
			TARGET ${TZ_BASE_ADD_EXECUTABLE_TARGET}
		)

		target_link_libraries(${TZ_BASE_ADD_EXECUTABLE_TARGET} PUBLIC ${TZ_BASE_ADD_EXECUTABLE_BASE})
	")
endfunction()

function(tz_add_library)
	cmake_parse_arguments(
		TZ_ADD_LIBRARY
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)

	tz_base_add_library(
		TARGET ${TZ_ADD_LIBRARY_TARGET}
		BASE tz
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

	tz_base_add_executable(
		TARGET ${TZ_ADD_EXECUTABLE_TARGET}
		BASE tz
		SOURCES ${TZ_ADD_EXECUTABLE_SOURCES}
	)

endfunction()

function(tz_add_library)
	cmake_parse_arguments(
		TZ_ADD_LIBRARY
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)

	tz_base_add_library(
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

	tz_base_add_executable(
		TARGET ${TZ_ADD_EXECUTABLE_TARGET}
		BASE topaz
		SOURCES ${TZ_ADD_EXECUTABLE_SOURCES}
	)
endfunction()
