function(tz_add_test)
	cmake_parse_arguments(
		TZ_ADD_TEST
		""
		"TARGET"
		"SOURCES"
		${ARGN}
	)

	hdk_base_add_test(
		TARGET ${TZ_ADD_TEST_TARGET}
		BASE_NAME tz
		SOURCES ${TZ_ADD_TEST_SOURCES}
	)
endfunction()
hdk_base_add_test_harness(BASE_NAME tz)

tz_add_test(
	TARGET tz_initialise_test
	SOURCES tz_initialise_test.cpp
)
