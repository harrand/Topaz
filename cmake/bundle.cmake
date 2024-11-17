define_property(TARGET PROPERTY TOPAZ_BUNDLE_DEPENDENCIES)
function(topaz_bundle_files)
	cmake_parse_arguments(
		TOPAZ_BUNDLE_FILES
		""
		"TARGET"
		"FILES;DIRECTORIES"
		${ARGN}
	)

    # for each bundle directory, glob recursively and add everything as files.
    # if a new thing was added in the directory, will reconfigure thanks to CONFIGURE_DEPENDS
    foreach(dir IN LISTS TOPAZ_BUNDLE_FILES_DIRECTORIES)
        file(GLOB_RECURSE local_files CONFIGURE_DEPENDS RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/${dir}/*)
        list(APPEND TOPAZ_BUNDLE_FILES_FILES ${local_files})
    endforeach()

	# source dir: CMAKE_CURRENT_SOURCE_DIR
	# Loop over each file and replicate the directory structure in the binary directory
    set_target_properties(${TOPAZ_BUNDLE_FILES_TARGET} PROPERTIES TOPAZ_BUNDLE_DEPENDENCIES "${TOPAZ_BUNDLE_FILES_FILES}")
    set(counter 0)
    foreach(file IN LISTS TOPAZ_BUNDLE_FILES_FILES)
        # Ensure the file exists
        set(file_path ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        if(NOT EXISTS ${file_path})
            message(FATAL_ERROR "File '${file_path}' does not exist!")
        endif()

        # Mark the target to depend on the file
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${file}"
            DEPENDS "${file_path}"
            COMMAND ${CMAKE_COMMAND} -E copy "${file_path}" "${CMAKE_CURRENT_BINARY_DIR}/${file}"
            COMMENT "Copying ${file_path} to ${CMAKE_CURRENT_BINARY_DIR}"
        )

        set(bundle_target_name ${TOPAZ_BUNDLE_FILES_TARGET}_bundle${counter})
        add_custom_target(${bundle_target_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${file}")

        add_dependencies(${TOPAZ_BUNDLE_FILES_TARGET} ${bundle_target_name})
        MATH(EXPR counter "${counter}+1")
    endforeach()
endfunction()