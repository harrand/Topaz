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
        pass_builder.add_pass(tz::gl::RenderPassAttachment::Colour);
        tz::gl::RenderPass render_pass = device.create_render_pass(pass_builder);

        tz::gl::ShaderBuilder builder;
        builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\shader_test.vertex.glsl");
        builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\shader_test.fragment.glsl");
        tz::gl::Shader shader = device.create_shader(builder);

        std::array<int, 5> values{1, 2, 3, 4, 5};
        tz::gl::BufferResource int_resource{tz::gl::BufferData::FromArray<int>(values)};

        tz::gl::RendererBuilder renderer_builder;
        tz::gl::ResourceHandle int_handle = renderer_builder.add_resource(int_resource);
        renderer_builder.set_output(tz::window());
        renderer_builder.set_render_pass(render_pass);
        renderer_builder.set_shader(shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        tz_assert(renderer.get_input() == nullptr, "Renderer wrongly thinks it has an input");

        // Ensure resources make sense.
        tz::gl::IResource* same_int_resource = renderer.get_resource(int_handle);
        tz_assert(same_int_resource != nullptr, "Renderer failed to process resource");
        tz_assert(same_int_resource != &int_resource, "Renderer does not clone resource");
        {
            auto byte_span = same_int_resource->get_resource_bytes();
            auto* values_again = reinterpret_cast<const int*>(byte_span.data());
            tz_assert(byte_span.size_bytes() == sizeof(values), "Renderer resource had unexpected byte span size. Expected %zu, got %zu.", sizeof(values), byte_span.size_bytes());
            for(std::size_t i = 0; i < values.size(); i++)
            {
                tz_assert(values[i] == values_again[i], "Renderer static resource data does not match the resource provided in the RendererBuilder.");
            }
        }

        for(std::size_t i = 0; i < frame_number; i++)
        {
            tz::window().update();
            renderer.render();
        }
    }
    tz::terminate();
}