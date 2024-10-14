# Level 0 #

function(topaz_add_library)
	cmake_parse_arguments(
		TOPAZ_ADD_LIBRARY
		""
		"TARGET"
		"SOURCES;SHADERS;TEXT_FILES"
		${ARGN}
	)

	add_library(${TOPAZ_ADD_LIBRARY_TARGET} STATIC
		${TOPAZ_ADD_LIBRARY_SOURCES}
	)
	topaz_add_shader(
		TARGET ${TOPAZ_ADD_LIBRARY_TARGET}
		SHADERS
			${TOPAZ_ADD_LIBRARY_SHADERS}
	)
	add_text(
		TARGET ${TOPAZ_ADD_LIBRARY_TARGET}
		INPUT_DIR ${PROJECT_SOURCE_DIR}
		OUTPUT_DIR ${PROJECT_BINARY_DIR}
		TEXT_FILES ${TOPAZ_ADD_LIBRARY_TEXT_FILES}
	)
endfunction()

function(topaz_add_executable)
	cmake_parse_arguments(
		TOPAZ_ADD_EXECUTABLE
		""
		"TARGET"
		"SOURCES;SHADERS"
		${ARGN}
	)

	add_executable(${TOPAZ_ADD_EXECUTABLE_TARGET}
		${TOPAZ_ADD_EXECUTABLE_SOURCES}
	)
	topaz_add_shader(
		TARGET ${TOPAZ_ADD_EXECUTABLE_TARGET}
		SHADERS
			${TOPAZ_ADD_EXECUTABLE_SHADERS}
	)
	add_text(
		TARGET ${TOPAZ_ADD_EXECUTABLE_TARGET}
		INPUT_DIR ${PROJECT_SOURCE_DIR}
		OUTPUT_DIR ${PROJECT_BINARY_DIR}
		TEXT_FILES ${TOPAZ_ADD_EXECUTABLE_TEXT_FILES}
	)
endfunction()