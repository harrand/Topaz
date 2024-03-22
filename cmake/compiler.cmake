# Compiler Config
function(setup_compiler)
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		configure_msvc()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		configure_clang()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		configure_gcc()
	endif()
endfunction()

function(configure_msvc)
	# TODO	
endfunction()

function(configure_gnu_like)
	# GNU-like compiler. i.e gcc + clang, but not msvc or clang-cl.
	# TODO	
endfunction()

function(configure_clang)
	# Either clang or clang-cl.
	#target_compile_options(topaz PUBLIC -Xclang -fskip-odr-check-in-gmf # Added at https://github.com/llvm/llvm-project/pull/79959 to workaround false-positive ODR violations from global module fragment (see: https://github.com/llvm/llvm-project/issues/78850))

	if(CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
		# clang-cl (or clang against MSVC). Just pretend we're msvc.
		configure_msvc()
		return()
	endif()
	# Code below runs if we're clang proper.
	configure_gnu_like()
endfunction()

function(configure_gcc)
	configure_gnu_like()
	# TODO	
endfunction()
