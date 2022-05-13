include (CMakeParseArguments)

function(add_text)
	set(TEXTC_EXECUTABLE_PATH $<TARGET_FILE:textc>)
	cmake_parse_arguments(
		ADD_TEXT
		""
		"TARGET"
		"TEXT_FILES"
		${ARGN}
	)
	
	foreach(TEXT ${ADD_TEXT_TEXT_FILES})
		set(text_path ${PROJECT_SOURCE_DIR}/${TEXT})
		set(output_name ${TEXT}.hpp)
		set(output_path ${PROJECT_BINARY_DIR}/${output_name})
		cmake_path(GET output_path PARENT_PATH text_dirname)
		#message(FATAL_ERROR "text: ${TEXT}, \n text_path: ${text_path}")
		add_custom_command(
			OUTPUT ${output_path}
			COMMENT "TEXTC: Bundling ${TEXT} -> ${output_name}"
			COMMAND "${TEXTC_EXECUTABLE_PATH}" ${text_path} > ${output_path}
			DEPENDS textc ${text_path}
			IMPLICIT_DEPENDS CXX
			VERBATIM
		)

		set_source_files_properties(${output_path} PROPERTIES GENERATED TRUE)
		target_sources(${ADD_TEXT_TARGET} PRIVATE ${output_path})
		# FInally, create a target represening the header so we can depend on it.
		get_filename_component(text_name ${output_path} NAME_WLE)
		set(text_header_library ${ADD_TEXT_TARGET}_${text_name})
		add_library(${text_header_library} INTERFACE)
		target_include_directories(${text_header_library} INTERFACE ${text_dirname})
		target_link_libraries(${ADD_TEXT_TARGET} PRIVATE ${text_header_library})
	endforeach()
endfunction()
