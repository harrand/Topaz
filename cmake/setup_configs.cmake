function(configure_opengl target)
    target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=0 -DTZ_OGL=1)
    
    target_link_libraries(${target} PUBLIC glad)
endfunction()

function(configure_vulkan target)
    target_compile_definitions(${target} PUBLIC -DTZ_VULKAN=1 -DTZ_OGL=0)

    message(STATUS "configure_vulkan(${target}): $VK_SDK_PATH == $ENV{VK_SDK_PATH}")
    target_link_directories(${target} PUBLIC "$ENV{VK_SDK_PATH}/Lib")
    target_include_directories(${target} PUBLIC "$ENV{VK_SDK_PATH}/include")
    target_link_libraries(${target} PUBLIC vulkan-1 vma)
endfunction()

function(configure_debug target)
    target_compile_definitions(${target} PUBLIC -DTZ_DEBUG=1 -DTZ_PROFILE=0)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PUBLIC /Zi)
    else()
        target_compile_options(${target} PUBLIC -g)
    endif()
endfunction()

function(configure_release target)
    target_compile_definitions(${target} PUBLIC -DTZ_DEBUG=0 -DTZ_PROFILE=0)
    target_compile_options(${target} PRIVATE -O3)
endfunction()

function(configure_profile target)
    target_compile_definitions(${target} PUBLIC -DTZ_DEBUG=0 -DTZ_PROFILE=1 -DTRACY_ENABLE)
    target_compile_options(${target} PRIVATE -O3)
    target_link_libraries(${target} PUBLIC TracyClient)
endfunction()