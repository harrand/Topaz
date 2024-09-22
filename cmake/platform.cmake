function(setup_platform)
	if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
		configure_windows()
	elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
		configure_linux()
	else()
		message(FATAL_ERROR "Building on unknown OS \"${CMAKE_HOST_SYSTEM_NAME}\". Only supoprt \"Windows\" or \"Linux\".")
	endif()
endfunction()

function(configure_windows)
	# TODO
	set(default_render_api "vulkan")
endfunction()

function(configure_linux)
	# TODO
	set(default_render_api "vulkan")
endfunction()