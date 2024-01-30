function(configure_windows target)
	target_link_libraries(${target} PUBLIC OpenGL32 dwmapi winmm)
endfunction()

function(configure_linux target)
	# Linux via X11
	message(WARNING "Support for Linux is not yet fully implemented")
	find_package(X11 REQUIRED)
	# Prefer new OGL over legacy.
	cmake_policy(SET CMP0072 NEW)
	find_package(OpenGL REQUIRED)
	# Xrandr for retrieving monitor info.
	target_link_libraries(${target} PUBLIC X11 X11::Xrandr OpenGL::GL)
endfunction()
