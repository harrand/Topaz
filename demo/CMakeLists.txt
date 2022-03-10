function(add_demo)
	cmake_parse_arguments(
		ADD_DEMO
		""
		"TARGET"
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
endfunction()

add_subdirectory(core)
add_subdirectory(gl)
