cmake_minimum_required(VERSION 3.9)

add_subdirectory(glfw) # glfw pre-3.3.5 https://github.com/glfw/glfw -- commit e7758c506dfcf258566f866c761c1b6bd7d98011
add_subdirectory(glad)# glad https://github.com/Dav1dde/glad/commits/master -- commit 00fb9287757364737277eec507e70f5fca743040
add_subdirectory(debugbreak)# debugbreak https://github.com/scottt/debugbreak -- commit 5dcbe41d2bd4712c8014aa7e843723ad7b40fd74
add_subdirectory(VulkanMemoryAllocator)# https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/tree/cc47fcf8672be670d7d6b3a084fdc9afdd08bc0b
add_subdirectory(tracy)