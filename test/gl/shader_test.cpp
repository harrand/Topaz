#include "core/tz.hpp"
#include "core/assert.hpp"
#include "gl/device.hpp"
#include "gl/shader.hpp"

int main()
{
    tz::initialise({"tz_shader_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
    {
        // Simply create some basic shaders. Don't try and render or anything.
        tz::gl::Device device{tz::gl::DeviceBuilder{}};

        tz::gl::ShaderBuilder builder;
        builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\shader_test.vertex.glsl");
        builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\shader_test.fragment.glsl");
        tz::gl::Shader shader = device.create_shader(builder);

        #if TZ_VULKAN
            const tz::gl::ShaderMeta& meta = shader.vk_get_meta();
            tz_assert(meta.resource_types.at(0) == "ubo", "Shader failed to generate correct meta");
            tz_assert(meta.resource_types.at(1) == "ssbo", "Shader failed to generate correct meta")
        #elif TZ_OGL
            tz_error("Not yet implemented (OGL)");
        #endif
    }
    tz::terminate();
}