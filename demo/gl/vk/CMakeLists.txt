add_demo(
    TARGET vk_init_demo
    SOURCE_FILES vk_init_demo.cpp
)

add_demo(
    TARGET vk_device_demo
    SOURCE_FILES vk_device_demo.cpp
)

add_demo(
    TARGET vk_window_demo
    SOURCE_FILES vk_window_demo.cpp
)

add_demo(
    TARGET vk_squares_demo
    SOURCE_FILES vk_squares_demo.cpp
    SHADER_SOURCES
        demo/gl/vk/basic.vertex.glsl
        demo/gl/vk/basic.fragment.glsl
)