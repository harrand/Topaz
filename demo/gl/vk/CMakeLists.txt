register_demo(vk_init_demo vk_init_demo.cpp)
register_demo(vk_device_demo vk_device_demo.cpp)
register_demo(vk_window_demo vk_window_demo.cpp)

add_executable(vk_squares_demo
    vk_squares_demo.cpp)
add_shader(TARGET vk_squares_demo
    SHADERS
    demo/gl/vk/basic.vertex.glsl
    demo/gl/vk/basic.fragment.glsl)
target_link_libraries(vk_squares_demo PRIVATE TZVK)