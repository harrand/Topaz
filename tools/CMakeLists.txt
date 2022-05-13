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
	STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" offset_dir ${CMAKE_CURRENT_SOURCE_DIR})

	set(prepended_text_list "")
	foreach(cur_text ${ADD_TOOL_TEXT_FILES})
		list(APPEND prepended_text_list "${offset_dir}/${cur_text}")
	endforeach()

message(WARNING "target: ${ADD_TOOL_TARGET} \n offset_dir: ${offset_dir} \n before: ${ADD_TOOL_TEXT_FILES} \n after: ${prepended_text_list}")
	add_text(
		TARGET ${ADD_TOOL_TARGET}
		TEXT_FILES ${prepended_text_list}
	)
endfunction()

add_subdirectory(textc)
add_subdirectory(tzslc)
