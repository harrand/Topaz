add_tz_test(NAME tz_allocator_test
	SOURCE_FILES allocator_test.cpp
	)

add_tz_test(NAME tz_callback_test
	SOURCE_FILES callback_test.cpp
	)

add_tz_test(NAME tz_clone_test
	SOURCE_FILES clone_test.cpp
	)

add_tz_test(NAME tz_initialise_test
	SOURCE_FILES initialise_test.cpp
	)

add_tz_test(NAME tz_job_test
	SOURCE_FILES job_test.cpp
	)

add_tz_test(NAME tz_matrix_test
	SOURCE_FILES matrix_test.cpp
	)

add_tz_test(NAME tz_time_test
	SOURCE_FILES time_test.cpp
	)

add_tz_test(NAME tz_text_test
	SOURCE_FILES text_test.cpp
	TEXT_FILES core/test_file.txt
	)

add_tz_test(NAME tz_transform_hierarchy_test
	SOURCE_FILES transform_hierarchy_test.cpp
	)

add_tz_test(NAME tz_types_test
	SOURCE_FILES types_test.cpp
	)

add_tz_test(NAME tz_types_test2
	SOURCE_FILES types_test2.cpp
	)

add_tz_test(NAME tz_vector_test
	SOURCE_FILES vector_test.cpp
	)
