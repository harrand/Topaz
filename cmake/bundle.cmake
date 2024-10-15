function(topaz_bundle_files)
	cmake_parse_arguments(
		TOPAZ_BUNDLE_FILES
		""
		"TARGET"
		"FILES"
		${ARGN}
	)

	# source dir: CMAKE_CURRENT_SOURCE_DIR
	# binary dir: $<TARGET_FILE_DIR:${TOPAZ_BUNDLE_FILES_TARGET}>
	set(binary_root $<TARGET_FILE_DIR:${TOPAZ_BUNDLE_FILES_TARGET}>)
	# Loop over each file and replicate the directory structure in the binary directory
    foreach(file IN LISTS TOPAZ_BUNDLE_FILES_FILES)
        # Get the relative path from the source directory
        set(file_path "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
        file(RELATIVE_PATH relative_path "${CMAKE_CURRENT_SOURCE_DIR}" ${file_path})

        # Construct the destination directory path in the binary directory
        set(destination_dir "$<TARGET_FILE_DIR:${TOPAZ_BUNDLE_FILES_TARGET}>/${relative_path}")

        # Ensure the directory exists
        add_custom_command(
            TARGET ${TOPAZ_BUNDLE_FILES_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TOPAZ_BUNDLE_FILES_TARGET}>/$$(dirname ${relative_path})"
            COMMENT "Creating directory $<TARGET_FILE_DIR:${TOPAZ_BUNDLE_FILES_TARGET}>/$(dirname ${relative_path})"
        )

        # Copy the file to the destination directory
        add_custom_command(
            TARGET ${TOPAZ_BUNDLE_FILES_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file_path}" "${destination_dir}"
            COMMENT "Copying ${file} to ${destination_dir}"
        )
    endforeach()
endfunction()