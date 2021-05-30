#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"

int main()
{
    constexpr std::size_t frame_number = 10;
    tz::initialise({"tz_renderer_test", tz::EngineInfo::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::HiddenWindowApplication);
    {
        // Simply create some basic shaders. Don't try and render or anything.
        tz::gl::Device device{tz::gl::DeviceBuilder{}};

        tz::gl::RenderPassBuilder pass_builder;
        pass_builder.add_pass(tz::gl::RenderPassAttachment::ColourDepth);
        tz::gl::RenderPass render_pass = device.create_render_pass(pass_builder);

        tz::gl::ShaderBuilder builder;
        builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\shader_test.vertex.glsl");
        builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\shader_test.fragment.glsl");
        tz::gl::Shader shader = device.create_shader(builder);

        tz::gl::RendererBuilder renderer_builder;
        renderer_builder.set_output(tz::window());
        renderer_builder.set_render_pass(render_pass);
        renderer_builder.set_shader(shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        for(std::size_t i = 0; i < frame_number; i++)
        {
            tz::window().update();
            renderer.render();
        }
    }
    tz::terminate();
}