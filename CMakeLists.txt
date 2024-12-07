cmake_minimum_required(VERSION 3.28)

#####################
## TOPAZ - VERSION ##
#####################

configure_file(version.txt version_txt_dummy.xml)
file(READ version.txt TZ_VERSION_TXT)
string(REGEX MATCH "[0-9]+\.[0-9]+" TZ_VERSION ${TZ_VERSION_TXT})

#####################
## TOPAZ - PROJECT ##
#####################

project(topaz
	VERSION ${TZ_VERSION}
	DESCRIPTION "C++20 Game Engine"
)

# C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(cmake/topaz.cmake)
add_subdirectory(lib)
add_subdirectory(tools)
include(cmake/shader.cmake)
include(cmake/bundle.cmake)
include(cmake/package.cmake)

####################
## TOPAZ - TARGET ##
####################

topaz_add_library(
	TARGET topaz
	SOURCES	
		src/tz/topaz.cpp
		src/tz/core/error.cpp
		src/tz/core/job.cpp
		src/tz/core/lua.cpp
		src/tz/core/time.cpp
		src/tz/core/vector.cpp
		src/tz/core/matrix.cpp
		src/tz/core/quaternion.cpp
		src/tz/core/trs.cpp
		src/tz/core/hier.cpp
		src/tz/gpu/rhi_vulkan.cpp
		src/tz/os/impl_win32.cpp
		src/tz/io/image.cpp
		src/tz/ren/quad.cpp
	SHADERS
		src/tz/ren/quad.vertex.tzsl
		src/tz/ren/quad.fragment.tzsl
	DEFINE_PACKAGE
)

target_include_directories(topaz PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(topaz PRIVATE VulkanMemoryAllocator concurrentqueue stb_image lua)
target_link_libraries(topaz PUBLIC textc_lib)

######################
## TOPAZ - SETTINGS ##
######################

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(cmake/build_config.cmake)
setup_build_config()
include(cmake/compiler.cmake)
setup_compiler()
include(cmake/platform.cmake)
setup_platform()
include(cmake/render_api.cmake)
setup_render_api()

###################
## TOPAZ - TESTS ##
###################

include(CTest)
add_subdirectory(test)

###################
## TOPAZ - DEMOS ##
###################

add_subdirectory(demo)