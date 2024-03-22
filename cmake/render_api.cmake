function(setup_render_api)
	# Use the render-api we were told.
	if(${TOPAZ_RENDER_API} MATCHES "vulkan")
		configure_vulkan()
		return()
	elseif(${TOPAZ_RENDER_API})
		message(FATAL_ERROR "Render-api \"${TOPAZ_RENDER_API}\" specified, but we do not recognise that. We support: \"vulkan\".")
	endif()

	# We weren't told a render-api. Fallback to a default (platform should've specified this).
	message(WARNING "No render-api (\${TOPAZ_RENDER_API}) specified. Falling back to default render-api for this platform.")
	if(${default_render_api} MATCHES "vulkan")
		configure_vulkan()
	else()
		message(FATAL_ERROR "Default render-api \"${default_render_api}\" for this platform is not supported.")
	endif()
endfunction()

function(configure_vulkan)
	find_package(Vulkan REQUIRED)
	target_link_libraries(topaz PRIVATE Vulkan::Vulkan)
endfunction()
