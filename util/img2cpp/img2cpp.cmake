function(add_image image_relative_path)
    set(img2cpp_path "${PROJECT_SOURCE_DIR}/build/debug/util/img2cpp/img2cpp")
    set(output_file_path "${PROJECT_SOURCE_DIR}/${image_relative_path}.cpp")
    message(STATUS "Adding image at ${image_relative_path}. Expected output = ${output_file_path}")
    
    execute_process(COMMAND ${img2cpp_path} ${image_relative_path}
    OUTPUT_FILE "${output_file_path}"
    RESULT_VARIABLE IMG2CPP_RESULT_CODE
    )

    if(${IMG2CPP_RESULT_CODE} LESS 0)
        message(FATAL_ERROR "Failed to run img2cpp on \"${PROJECT_SOURCE_DIR}/${image_relative_path}\"")
    endif()
endfunction()