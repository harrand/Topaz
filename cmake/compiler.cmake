# Compiler Config
function(setup_compiler)
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		configure_compiler_msvc()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		configure_compiler_clang()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		configure_compiler_gcc()
	endif()
endfunction()

function(configure_compiler_msvc)
	# TODO	
endfunction()

function(configure_compiler_gnu_like)
	# GNU-like compiler. i.e gcc + clang, but not msvc or clang-cl.
	# TODO	
endfunction()

function(configure_compiler_clang)
	# Either clang or clang-cl.
	if(CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
		# clang-cl. Just pretend we're msvc.
		configure_compiler_msvc()
		return()
	endif()
	# Code below runs if we're clang proper.
	configure_compiler_gnu_like()
	# TODO	
endfunction()

function(configure_compiler_gcc)
	configure_compiler_gnu_like()
	# TODO	
endfunction()