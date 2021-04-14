function(register_demo target_name source_file)
    add_executable(${target_name}
        ${source_file})
    target_link_libraries(${target_name} PRIVATE TZVK)
endfunction()

register_demo(engine_info_demo engine_info_demo.cpp)