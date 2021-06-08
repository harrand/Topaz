#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/shader.hpp"

int main()
{
    tz::initialise({"tz_shader_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::HiddenWindowApplication);
    {
        // Simply create some basic shaders. Don't try and render or anything.
        tz::gl::Device device{tz::gl::DeviceBuilder{}};

        tz::gl::ShaderBuilder builder;
        builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\shader_test.vertex.glsl");
        builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\shader_test.fragment.glsl");
        tz::gl::Shader shader = device.create_shader(builder);
    }
    tz::terminate();
}