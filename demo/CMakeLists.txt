function(add_demo)
	cmake_parse_arguments(
		ADD_DEMO
		""
		"TARGET;CUSTOM_ICON"
		"SOURCE_FILES;SHADER_SOURCES"
		${ARGN}
	)

	add_executable(${ADD_DEMO_TARGET}
		${ADD_DEMO_SOURCE_FILES}
	)

	set_target_properties(${ADD_DEMO_TARGET} PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF)

	target_link_libraries(${ADD_DEMO_TARGET} PRIVATE topaz)
	add_shader(
		TARGET ${ADD_DEMO_TARGET}
		SHADERS ${ADD_DEMO_SHADER_SOURCES}
	)

	if(${TOPAZ_BUILDCONFIG} MATCHES "release")
		if(ADD_DEMO_CUSTOM_ICON)
			set(icon_path ${ADD_DEMO_CUSTOM_ICON})
		else()
			set(icon_path ${PROJECT_SOURCE_DIR}/doc/favicon.ico)
		endif()
		tz_set_target_meta(TARGET ${ADD_DEMO_TARGET}
			NAME "untitled"
			ICON_PATH ${icon_path})
	endif()
endfunction()

add_subdirectory(core)
add_subdirectory(gl)
