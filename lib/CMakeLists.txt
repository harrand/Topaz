cmake_minimum_required(VERSION 3.9)

add_subdirectory(glfw) # glfw 3.3.3
add_subdirectory(glad)# glad https://github.com/Dav1dde/glad/commits/master -- commit 00fb9287757364737277eec507e70f5fca743040
add_subdirectory(debugbreak)# debugbreak https://github.com/scottt/debugbreak -- commit 5d620814b1a240bbdf50c1dceed1eaf5a3fefce8
add_subdirectory(VulkanMemoryAllocator)# https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/tree/cc47fcf8672be670d7d6b3a084fdc9afdd08bc0b
add_subdirectory(stb_image)