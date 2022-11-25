function(configure_common target)
	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		# GCC/Clang options
		target_compile_options(${target} PRIVATE -Wall -Wextra -Werror -pedantic-errors)
		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			# Clang-only options
			# Use GNU's `, __VA_ARGS__` extension (used for hdk::assert, hdk::error, hdk::report)
			target_compile_options(${target} PRIVATE -Wno-gnu-zero-variadic-macro-arguments)
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
			# GCC-only options
			target_compile_options(${target} PRIVATE -Wshadow=local)
		endif()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		# MSVC-only options
	endif()
	target_compile_definitions(${target} PUBLIC -DTZ_VERSION=\"${TZ_VERSION}\" -DTZ_SILENCED=0 -DGLFW_INCLUDE_NONE)

endfunction()

function(configure_opengl target)
	target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=0 -DTZ_OGL=1)
	
	target_link_libraries(${target} PUBLIC glad)
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
