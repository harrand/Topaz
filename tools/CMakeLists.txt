function(add_tool)
	cmake_parse_arguments(
		ADD_TOOL
		""
		"TARGET"
		"SOURCE_FILES;TEXT_FILES"
		${ARGN}
	)

	add_executable(${ADD_TOOL_TARGET}
		${ADD_TOOL_SOURCE_FILES})

	set_target_properties(${ADD_TOOL_TARGET} PROPERTIES
		CXX_STANDARD ${TZ_CXX_STANDARD}
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF)

	target_link_libraries(${ADD_TOOL_TARGET} PRIVATE topaz)
	STRING(REGEX REPLACE "^${PROJECT_SOURCE_DIR}/" "" offset_dir ${CMAKE_CURRENT_SOURCE_DIR})

	tz_append_list(
		PREFIX "${offset_dir}/"
		ELEMENTS ${ADD_TOOL_TEXT_FILES}
		OUTPUT prepended_text_list
	)

	add_text(
		TARGET ${ADD_TOOL_TARGET}
		INPUT_DIR ${PROJECT_SOURCE_DIR}
		OUTPUT_DIR ${PROJECT_BINARY_DIR}
		TEXT_FILES ${prepended_text_list}
	)
endfunction()

add_subdirectory(tzslc)
