function(configure_opengl target)
    target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=0 -DTZ_OGL=1)
    
    target_link_libraries(${target} PUBLIC glad)
endfunction()

function(configure_vulkan target)
    target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=0)

    message(STATUS "configure_vulkan(${target}): $VK_SDK_PATH == $ENV{VK_SDK_PATH}")
    target_link_directories(${target} PUBLIC "$ENV{VK_SDK_PATH}/Lib")
    target_include_directories(${target} PUBLIC "$ENV{VK_SDK_PATH}/include")
    target_link_libraries(${target} PUBLIC vulkan-1)
endfunction()

function(configure_debug target)
    target_compile_definitions(${target} PUBLIC -DTZ_DEBUG=1)
    target_compile_options(${target} PRIVATE -g)
endfunction()

function(configure_release target)
    target_compile_definitions(${target} PUBLIC -DTZ_DEBUG=0)
    target_compile_options(${target} PRIVATE -O3)
endfunction()