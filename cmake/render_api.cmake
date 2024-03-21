function(setup_render_api)
	if(${default_render_api} MATCHES "vulkan")
		configure_vulkan()
	else()
		message(FATAL_ERROR "Default render-api \"${default_render_api}\" for this platform is not supported.")
	endif()
endfunction()

function(configure_vulkan)
	# TODO	
endfunction()