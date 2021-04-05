cmake_minimum_required(VERSION 3.9)

add_subdirectory(glfw)# glfw 3.3.3
add_subdirectory(glad)# glad https://github.com/Dav1dde/glad/commits/master -- commit 00fb9287757364737277eec507e70f5fca743040
add_subdirectory(stb_image)
add_subdirectory(assimp)# assimp 5.0.1
add_subdirectory(imgui)# imgui 1.75 release
add_subdirectory(debugbreak)# debugbreak https://github.com/scottt/debugbreak -- commit 5d620814b1a240bbdf50c1dceed1eaf5a3fefce8