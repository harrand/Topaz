cmake_minimum_required(VERSION 3.28)

#####################
## TOPAZ - VERSION ##
#####################

configure_file(version.txt version_txt_dummy.xml)
file(READ version.txt TZ_VERSION_TXT)
string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" TZ_VERSION ${TZ_VERSION_TXT})

#####################
## TOPAZ - PROJECT ##
#####################

project(topaz
	VERSION ${TZ_VERSION}
	DESCRIPTION "C++20 Game Engine"
)

# Note: The following are global set variables instead of target properties.
# This is because *everything* should be std C++20 with modules support, including tests, applications, demos etc...
# Allow non-module sources to still import modules.
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
# C++20
set(CMAKE_CXX_STANDARD 20)
# C++20 is a hard-requirement; **only** support C++20.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler-specific extensions.
set(CMAKE_CXX_EXTENSIONS OFF)

####################
## TOPAZ - TARGET ##
####################

add_library(topaz STATIC)
target_sources(topaz
	PUBLIC
	FILE_SET CXX_MODULES FILES
		# Module Sources
		src/tz/topaz.cppm
		src/tz/debug/error.cppm
	PRIVATE
		# Non-Module Sources
		src/tz/deleteme.cpp
)

######################
## TOPAZ - SETTINGS ##
######################

# Build Config specific compiler settings (Debug/Release).
include(cmake/config.cmake) # setup_build_config
setup_build_config()
# Config-Independent compiler settings, e.g warnings.
include(cmake/compiler.cmake)
setup_compiler()
# Platform Settings (Windows/Linux)
include(cmake/platform.cmake)
setup_platform()
# Render API Settings (Vulkan)
include(cmake/render_api.cmake)
setup_render_api()

###################
## TOPAZ - TESTS ##
###################

include(CTest)
add_subdirectory(test)