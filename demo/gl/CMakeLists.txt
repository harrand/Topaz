add_subdirectory(vk)

add_demo(
    TARGET tz_triangle_demo
    SOURCE_FILES tz_triangle_demo.cpp
    SHADER_SOURCES
        demo/gl/triangle_demo.vertex.glsl
        demo/gl/triangle_demo.fragment.glsl
)

add_demo(
    TARGET tz_dynamic_triangle_demo
    SOURCE_FILES tz_dynamic_triangle_demo.cpp
    SHADER_SOURCES
        demo/gl/triangle_demo.vertex.glsl
        demo/gl/triangle_demo.fragment.glsl
)