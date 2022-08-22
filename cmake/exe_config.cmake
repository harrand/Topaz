function(tz_set_target_meta)
	cmake_parse_arguments(
		SET_TARGET_META
		""
		"TARGET;NAME;ICON_PATH"
		""
		${ARGN}
	)

	if(WIN32)
		# Create a rc file in the output directory and then add it as a source.
		set(rc_path ${PROJECT_BINARY_DIR}/${SET_TARGET_META_TARGET}_meta.rc)
		add_custom_command(OUTPUT ${rc_path}
			COMMENT "Creating RC file for ${SET_TARGET_META_TARGET}. Icon path = ${SET_TARGET_META_ICON_PATH}"
			COMMAND echo IDI_ICON1 ICON    DISCARDABLE     "${SET_TARGET_META_ICON_PATH}" > ${rc_path}
			VERBATIM
			)
		target_sources(${SET_TARGET_META_TARGET} PRIVATE ${rc_path})
		set_source_files_properties(${rc_path} PROPERTIES GENERATED TRUE)
	endif()
endfunction()
