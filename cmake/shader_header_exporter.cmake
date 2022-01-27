include (CMakeParseArguments)

macro(export_shader_header)
	cmake_parse_arguments(
		EXPORT_SHADER_HEADER
		""
		"FILE_NAME"
		""
		${ARGN}
	)

	cmake_path(GET EXPORT_SHADER_HEADER_FILE_NAME PARENT_PATH shader_dirname)
	get_filename_component(shader_name ${EXPORT_SHADER_HEADER_FILE_NAME} NAME_WLE)
	set(shader_header_name ${shader_dirname}/${shader_name}.hpp)
	set(shader_header_target_name ${ADD_SHADER_TARGET}_${shader_name})

	add_custom_command(
		OUTPUT ${shader_header_name}
		COMMENT "Exporting Shader Binary of \"${shader_name}\" to the path \"${shader_header_name}\" as an import header."
		COMMAND "${TZSLC_EXECUTABLE_PATH}" ${EXPORT_SHADER_HEADER_FILE_NAME} -gen_header -o ${shader_header_name}
		DEPENDS tzslc ${EXPORT_SHADER_HEADER_FILE_NAME}
		IMPLICIT_DEPENDS CXX ${EXPORT_SHADER_HEADER_FILE_NAME}
		VERBATIM)
	set_source_files_properties(${shader_header_name} PROPERTIES GENERATED TRUE)
	target_sources(${ADD_SHADER_TARGET} PRIVATE ${shader_header_name})
	# Finally, create a target representing the generated header and make the target dependent on it so it can include it!
	set(shader_header_library ${shader_header_target_name})
	add_library(${shader_header_library} INTERFACE)
	target_include_directories(${shader_header_library} INTERFACE ${shader_dirname})
	target_link_libraries(${ADD_SHADER_TARGET} PRIVATE ${shader_header_library})
endmacro()
