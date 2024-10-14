function(topaz_add_demo)
	cmake_parse_arguments(
		TOPAZ_ADD_DEMO
		""
		"TARGET"
		"SOURCES;SHADERS;TEXT_FILES"
		${ARGN}
	)	

	topaz_add_executable(
		TARGET ${TOPAZ_ADD_DEMO_TARGET}
		SOURCES
			${TOPAZ_ADD_DEMO_SOURCES}
		SHADERS
			${TOPAZ_ADD_DEMO_SHADERS}
		TEXT_FILES
			${TOPAZ_ADD_DEMO_TEXT_FILES}
	)
	# All demos link against topaz.
	target_link_libraries(${TOPAZ_ADD_DEMO_TARGET} PRIVATE topaz)
endfunction()

add_subdirectory(tz)