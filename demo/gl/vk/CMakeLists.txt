register_demo(vk_init_demo vk_init_demo.cpp)
register_demo(vk_device_demo vk_device_demo.cpp)
register_demo(vk_window_demo vk_window_demo.cpp)

add_executable(vk_triangle_demo
    vk_triangle_demo.cpp)
add_shader(TARGET vk_triangle_demo
    SHADERS
    demo/gl/vk/impl/basic.vertex.glsl
    demo/gl/vk/impl/basic.fragment.glsl)
target_link_libraries(vk_triangle_demo PRIVATE TZVK)