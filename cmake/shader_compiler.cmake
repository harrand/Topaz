include(CMakeParseArguments)

macro(add_shader_vulkan)
    # example: C:\VulkanSDK\1.2.170.0\Bin\glslc.exe
    set(VK_BIN_DIR $ENV{VK_SDK_PATH}/Bin)
    set(VK_GLSLC_EXECUTABLE_PATH ${VK_BIN_DIR}/glslc.exe)

    set(shader_path ${PROJECT_SOURCE_DIR}/${SHADER})
    set(spv_path ${PROJECT_BINARY_DIR}/${SHADER}.spv)

    add_custom_command(
        OUTPUT ${spv_path}
        COMMENT "VK: Building ${spv_path}"
        COMMAND ${VK_GLSLC_EXECUTABLE_PATH} -o ${spv_path} ${shader_path}
        DEPENDS ${shader_path}
        IMPLICIT_DEPENDS CXX ${shader_path}
        VERBATIM)

    set_source_files_properties(${spv_path} PROPERTIES GENERATED TRUE)
    target_sources(${ADD_SHADER_TARGET} PRIVATE ${spv_path})
endmacro()

macro(add_shader_opengl)
    set(shader_path ${PROJECT_SOURCE_DIR}/${SHADER})
    set(output_path ${PROJECT_BINARY_DIR}/${SHADER})

    add_custom_command(
        OUTPUT ${output_path}
        COMMENT "OGL: Building ${output_path}"
        COMMAND ${CMAKE_COMMAND} -E copy ${shader_path} ${output_path}
    )

    set_source_files_properties(${output_path} PROPERTIES GENERATED TRUE)
    target_sources(${ADD_SHADER_TARGET} PRIVATE ${output_path})
endmacro()

function(add_shader)
    
    cmake_parse_arguments(
        ADD_SHADER
        ""
        "TARGET"
        "SHADERS"
        ${ARGN}
    )

    foreach(SHADER ${ADD_SHADER_SHADERS})
        if(${TOPAZ_OGL})
            add_shader_opengl()
        elseif(${TOPAZ_VULKAN})
            add_shader_vulkan()
        endif()
    endforeach()
endfunction()