include(CMakeParseArguments)

macro(add_shader_vulkan)
    # example: C:\VulkanSDK\1.2.170.0\Bin\glslc.exe
    set(VK_BIN_DIR $ENV{VK_SDK_PATH}/Bin)
    set(VK_GLSLC_EXECUTABLE_PATH ${VK_BIN_DIR}/glslc.exe)

    set(shader_path ${PROJECT_SOURCE_DIR}/${SHADER})
    set(processed_shader_path ${PROJECT_BINARY_DIR}/${SHADER}.glsl)

    add_custom_command(
        OUTPUT ${processed_shader_path}
        COMMENT "TZSLC_VK: Building ${processed_shader_path}"
        COMMAND "${TZSLC_EXECUTABLE_PATH}" ${shader_path} -mall -o ${processed_shader_path}
        DEPENDS ${tzslc} ${shader_path}
        IMPLICIT_DEPENDS CXX ${shader_path}
        VERBATIM
    )
    set(spv_path ${PROJECT_BINARY_DIR}/${SHADER}.spv)

    add_custom_command(
        OUTPUT ${spv_path}
        COMMENT "VK: Building ${spv_path}"
        COMMAND ${VK_GLSLC_EXECUTABLE_PATH} -o ${spv_path} ${processed_shader_path}
        DEPENDS ${processed_shader_path}
        IMPLICIT_DEPENDS CXX ${processed_shader_path}
        VERBATIM)

    set_source_files_properties(${processed_shader_path} ${spv_path} PROPERTIES GENERATED TRUE)
    add_dependencies(${ADD_SHADER_TARGET} tzslc)
    target_sources(${ADD_SHADER_TARGET} PRIVATE ${processed_shader_path} ${spv_path})
endmacro()

macro(add_shader_opengl)
    set(shader_path ${PROJECT_SOURCE_DIR}/${SHADER})
    set(output_path ${PROJECT_BINARY_DIR}/${SHADER}.glsl)

    add_custom_command(
        OUTPUT ${output_path}
        COMMENT "TZSLC_OGL: Building ${output_path}"
        COMMAND "${TZSLC_EXECUTABLE_PATH}" ${shader_path} -mall -o ${output_path}
        DEPENDS ${shader_path}
        IMPLICIT_DEPENDS CXX ${processed_shader_path}
        VERBATIM
    )

    set_source_files_properties(${output_path} PROPERTIES GENERATED TRUE)
    add_dependencies(${ADD_SHADER_TARGET} tzslc)
    target_sources(${ADD_SHADER_TARGET} PRIVATE ${output_path})
endmacro()

function(add_shader)
    set(TZSLC_EXECUTABLE_PATH ${PROJECT_BINARY_DIR}/tools/tzslc/tzslc)
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