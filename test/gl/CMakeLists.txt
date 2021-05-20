add_tz_test(NAME tz_shader_test
        SOURCE_FILES shader_test.cpp
        SHADER_SOURCES
            test/gl/shader_test.vertex.glsl
            test/gl/shader_test.fragment.glsl
        GRAPHICAL
        )