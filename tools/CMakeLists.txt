function(add_tool)
    cmake_parse_arguments(
        ADD_TOOL
        ""
        "TARGET"
        "SOURCE_FILES"
        ${ARGN}
    )

    add_executable(${ADD_TOOL_TARGET}
        ${ADD_TOOL_SOURCE_FILES})
    target_link_libraries(${ADD_TOOL_TARGET} PRIVATE topaz)
endfunction()

add_subdirectory(tzslc)