function(configure_common target)
	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		# GCC/Clang options
		message(STATUS "GNU|Clang")
		target_compile_options(${target} PRIVATE -Wall -Wextra -Werror -pedantic-errors)
		target_compile_options(${target} PUBLIC -Wno-format-security)
		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			message(STATUS "Clang")
			# Clang-only options
			# Use GNU's `, __VA_ARGS__` extension (used for tz::assert, tz::error, tz::report)
			target_compile_options(${target} PRIVATE -Wno-gnu-zero-variadic-macro-arguments)
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
			# GCC-only options
			message(STATUS "GNU")
		endif()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		message(STATUS "MSVC")
		# MSVC-only options
	endif()
	target_compile_definitions(${target} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(${target} PUBLIC -DGLFW_INCLUDE_NONE -DTZ_VERSION=0x${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}${PROJECT_VERSION_TWEAK})

endfunction()

function(configure_opengl target)
	target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=0 -DTZ_OGL=1)
endfunction()

function(configure_vulkan target)
	target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=0)

	message(STATUS "configure_vulkan(${target}): $VULKAN_SDK == $ENV{VULKAN_SDK}")
	if(WIN32)
		target_link_directories(${target} PUBLIC "$ENV{VULKAN_SDK}/Lib")
	elseif(UNIX)
		target_link_directories(${target} PUBLIC "$ENV{VULKAN_SDK}/lib")
	endif()
	target_include_directories(${target} PUBLIC "$ENV{VULKAN_SDK}/include")
	if(WIN32)
		target_link_libraries(${target} PUBLIC vulkan-1 tz_vma)
	elseif(UNIX)
		target_link_libraries(${target} PUBLIC vulkan tz_vma)
	endif()
endfunction()
