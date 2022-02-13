cmake_minimum_required(VERSION 3.9)

add_subdirectory(glfw)
add_subdirectory(glad)# Special glad from https://glad.dav1d.de/ (ogl 4.6 with bindless textures)
add_subdirectory(debugbreak)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(tracy)