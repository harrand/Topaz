add_tz_test(NAME tz_mesh_test
        SOURCE_FILES mesh_test.cpp
        GRAPHICAL
        )

add_tz_test(NAME tz_renderer_test
        SOURCE_FILES renderer_test.cpp
        SHADER_SOURCES
            test/gl/shader_test.vertex.glsl
            test/gl/shader_test.fragment.glsl
        GRAPHICAL
        )

add_tz_test(NAME tz_shader_test
        SOURCE_FILES shader_test.cpp
        SHADER_SOURCES
            test/gl/shader_test.vertex.glsl
            test/gl/shader_test.fragment.glsl
        GRAPHICAL
        )
