function(tz_append_list)
	cmake_parse_arguments(
		APPEND_LIST
		""
		"PREFIX;SUFFIX;OUTPUT"
		"ELEMENTS"
		${ARGN}
	)

	set(ret "")
	foreach(elem ${APPEND_LIST_ELEMENTS})
		list(APPEND ret "${APPEND_LIST_PREFIX}${elem}${APPEND_LIST_SUFFIX}")
	endforeach()
	set(${APPEND_LIST_OUTPUT} ${ret} PARENT_SCOPE)
endfunction()
