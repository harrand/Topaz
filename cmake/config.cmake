# Build Config
function(setup_build_config)
	set(topaz_default_build_config "debug")

	if(NOT DEFINED CMAKE_BUILD_TYPE)
		message(WARNING "Build Config `\$\{CMAKE_BUILD_TYPE\}` not specified - defaulting to `debug`.")
		set(CMAKE_BUILD_TYPE ${topaz_default_build_config})
	else()
		# Must match one of the values:

		if(${CMAKE_BUILD_TYPE} MATCHES "debug")
			# Debug (debug symbols, minimal optimisation, asserts enabled). Use this if superdebug gets too slow for you.
			configure_debug()
		elseif(${CMAKE_BUILD_TYPE} MATCHES "release")
			# Release (a build you can ship)
			configure_release()
		elseif(${CMAKE_BUILD_TYPE} MATCHES "profile")
			# Profile (similar to release, but profiling instrumentation is not compiled out. Use this to profile performance)
			configure_profile()
		else()
			message(FATAL_ERROR "Unknown build type (CMAKE_BUILD_TYPE) \"${CMAKE_BUILD_TYPE}\"")
		endif()

	endif()
endfunction()

function(configure_debug)
	# TODO
	target_compile_definitions(topaz PRIVATE -DTOPAZ_DEBUG=1)
endfunction()

function(configure_release)
	# TODO
	target_compile_definitions(topaz PRIVATE -DTOPAZ_DEBUG=0)
endfunction()

function(configure_profile)
	# TODO
	target_compile_definitions(topaz PRIVATE -DTOPAZ_DEBUG=0)
endfunction()
