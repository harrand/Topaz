add_subdirectory(vk)

add_demo(
    TARGET tz_triangle_demo
    SOURCE_FILES tz_triangle_demo.cpp
    SHADER_SOURCES
        demo/gl/triangle_demo.vertex.tzsl
        demo/gl/triangle_demo.fragment.tzsl
)

add_demo(
    TARGET tz_dynamic_triangle_demo
    SOURCE_FILES tz_dynamic_triangle_demo.cpp
    SHADER_SOURCES
        demo/gl/dynamic_triangle_demo.vertex.tzsl
        demo/gl/dynamic_triangle_demo.fragment.tzsl
)

add_demo(
    TARGET tz_multi_draw_demo
    SOURCE_FILES tz_multi_draw_demo.cpp
    SHADER_SOURCES
        demo/gl/dynamic_triangle_demo.vertex.tzsl
        demo/gl/dynamic_triangle_demo.fragment.tzsl
)