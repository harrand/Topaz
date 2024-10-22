set(TOPAZ_PACKAGE_BASE_DIR "${PROJECT_BINARY_DIR}/packages")
function(topaz_define_package)
	cmake_parse_arguments(
		TOPAZ_DEFINE_PACKAGE
		""
		"TARGET"
		""
		${ARGN}
	)

	set(dependency_list "")
	set(package_dir ${TOPAZ_PACKAGE_BASE_DIR}/${TOPAZ_DEFINE_PACKAGE_TARGET}.package)

	get_target_property(bundle_deps ${TOPAZ_DEFINE_PACKAGE_TARGET} TOPAZ_BUNDLE_DEPENDENCIES)
	# Create commands to copy all the bundled files to the package dir.
	if(NOT ${bundle_deps} STREQUAL "bundle_deps-NOTFOUND")
		foreach(bundle_file IN LISTS bundle_deps)
			set(bundle_file_path $<TARGET_FILE_DIR:${TOPAZ_DEFINE_PACKAGE_TARGET}>/${bundle_file})
			add_custom_command(
				OUTPUT "${package_dir}/${bundle_file}"
				DEPENDS ${bundle_file_path}
				COMMAND ${CMAKE_COMMAND} -E copy "${bundle_file_path}" "${package_dir}/${bundle_file}"
				COMMENT "Copying bundled file ${bundle_file} into ${TOPAZ_DEFINE_PACKAGE_TARGET}.package"
			)
			list(APPEND dependency_list "${package_dir}/${bundle_file}")
		endforeach()
	endif()
	# Make tar.package which copies the bundled files
	add_custom_target(${TOPAZ_DEFINE_PACKAGE_TARGET}.package
		ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${package_dir}
		DEPENDS ${TOPAZ_DEFINE_PACKAGE_TARGET} ${dependency_list}
	)
	# And also to copy over the executable/library.
	add_custom_command(TARGET ${TOPAZ_DEFINE_PACKAGE_TARGET}.package
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TOPAZ_DEFINE_PACKAGE_TARGET}> "${package_dir}"
		COMMAND_EXPAND_LISTS
	)
	# And finally, all dll dependencies
	# why use -t here when its not used anywhere else?
	# well TARGET_RUNTIME_DLLS could may well yield an empty list. copy without -t is malformed if you try to copy 0 things, but with -t its much better.
	# (note this only works for executable targets)
	get_target_property(target_type ${TOPAZ_DEFINE_PACKAGE_TARGET} TYPE)
	if(target_type STREQUAL "EXECUTABLE")
		add_custom_command(TARGET ${TOPAZ_DEFINE_PACKAGE_TARGET}.package
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy -t "${package_dir}" $<TARGET_RUNTIME_DLLS:${TOPAZ_DEFINE_PACKAGE_TARGET}>
			COMMAND_EXPAND_LISTS
		)
	endif()
endfunction()