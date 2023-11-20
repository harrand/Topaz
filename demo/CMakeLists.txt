function(add_demo)
	cmake_parse_arguments(
		ADD_DEMO
		""
		"TARGET;CUSTOM_ICON"
		"SOURCE_FILES;SHADER_SOURCES"
		${ARGN}
	)

	tz_add_executable(TARGET ${ADD_DEMO_TARGET}
		SOURCES ${ADD_DEMO_SOURCE_FILES})

	add_shader(
		TARGET ${ADD_DEMO_TARGET}
		SHADERS ${ADD_DEMO_SHADER_SOURCES}
	)

	if(${TOPAZ_BUILDCONFIG} MATCHES "release")
		if(ADD_DEMO_CUSTOM_ICON)
			set(icon_path ${ADD_DEMO_CUSTOM_ICON})
		else()
			get_target_property(TOPAZ_ROOT_DIR topaz SOURCE_DIR)
			set(icon_path ${TOPAZ_ROOT_DIR}/doc/favicon.ico)
		endif()
		tz_set_target_meta(TARGET ${ADD_DEMO_TARGET}
			NAME ${ADD_DEMO_TARGET}
			ICON_PATH ${icon_path})
	endif()
endfunction()

add_subdirectory(core)
add_subdirectory(gl)
add_subdirectory(ren)
