cmake_minimum_required(VERSION 3.9)

add_subdirectory(glad)# Special glad from https://glad.dav1d.de/ (ogl 4.6 with bindless textures)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(textc)
add_subdirectory(concurrentqueue)
add_subdirectory(debugbreak)
add_subdirectory(tracy)

add_library(imgui STATIC
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/lib/imgui")
target_compile_definitions(imgui PUBLIC "-DIMGUI_USER_CONFIG=\"../tz_imgui_config.hpp\"")

add_library(imgui_memory_editor INTERFACE)
target_include_directories(imgui_memory_editor INTERFACE imgui_club/imgui_memory_editor)

add_library(tz_vma STATIC
	tz_vma.cpp)
target_link_libraries(tz_vma PUBLIC vma)
if(WIN32)
	target_link_directories(tz_vma PUBLIC "$ENV{VULKAN_SDK}/Lib")
	target_link_libraries(tz_vma PRIVATE vulkan-1)
elseif(UNIX)
	target_link_directories(tz_vma PUBLIC "$ENV{VULKAN_SDK}/lib")
	target_link_libraries(tz_vma PRIVATE vulkan)
endif()
target_include_directories(tz_vma PUBLIC "$ENV{VULKAN_SDK}/include")
