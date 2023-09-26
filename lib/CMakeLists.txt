cmake_minimum_required(VERSION 3.9)

add_subdirectory(glad)# Special glad from https://glad.dav1d.de/ (ogl 4.6 with bindless textures)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(textc)
add_subdirectory(concurrentqueue)
add_subdirectory(json)
add_subdirectory(debugbreak)
add_subdirectory(tracy)

set(MSDFGEN_CORE_ONLY ON)
add_subdirectory(msdfgen)

add_library(imgui STATIC
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp

	imgui/misc/cpp/imgui_stdlib.cpp)
target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/lib/imgui")
target_compile_definitions(imgui PUBLIC "-DIMGUI_USER_CONFIG=\"../tz_imgui_config.hpp\"")

add_library(imgui_memory_editor INTERFACE)
target_include_directories(imgui_memory_editor INTERFACE imgui_club/imgui_memory_editor)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb)

add_library(tz_vma STATIC
	tz_vma.cpp)
target_link_libraries(tz_vma PUBLIC vma)

add_library(tz_lua STATIC
	lua/lapi.c
	lua/lcode.c
	lua/lctype.c
	lua/ldebug.c
	lua/ldo.c
	lua/ldump.c
	lua/lfunc.c
	lua/lgc.c
	lua/llex.c
	lua/lmem.c
	lua/lobject.c
	lua/lopcodes.c
	lua/lparser.c
	lua/lstate.c
	lua/lstring.c
	lua/ltable.c
	lua/ltm.c
	lua/lundump.c
	lua/lvm.c
	lua/lzio.c
	lua/lauxlib.c
	lua/lbaselib.c
	lua/lcorolib.c
	lua/ldblib.c
	lua/liolib.c
	lua/lmathlib.c
	lua/loadlib.c
	lua/loslib.c
	lua/lstrlib.c
	lua/ltablib.c
	lua/lutf8lib.c
	lua/linit.c

	)
target_include_directories(tz_lua PUBLIC "${PROJECT_SOURCE_DIR}/lib/lua")

if(WIN32)
	target_link_directories(tz_vma PUBLIC "$ENV{VULKAN_SDK}/Lib")
	target_link_libraries(tz_vma PRIVATE vulkan-1)
elseif(UNIX)
	target_link_directories(tz_vma PUBLIC "$ENV{VULKAN_SDK}/lib")
	target_link_libraries(tz_vma PRIVATE vulkan)
endif()
target_include_directories(tz_vma PUBLIC "$ENV{VULKAN_SDK}/include")
