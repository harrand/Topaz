function(configure_windows target)
	target_link_libraries(${target} PUBLIC OpenGL32 dwmapi)
endfunction()

function(configure_linux target)
	# Linux via X11
	message(WARNING "Support for Linux is not yet fully implemented")
	find_package(X11 REQUIRED)
	find_package(OpenGL REQUIRED)
	# Xrandr for retrieving monitor info.
	target_link_libraries(${target} PUBLIC X11 X11::Xrandr OpenGL::GL)
endfunction()
