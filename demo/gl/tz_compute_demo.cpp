#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/processor.hpp"
#include "gl/resource.hpp"
#include "gl/input.hpp"
#include "gl/shader.hpp"

float get_aspect_ratio()
{
    return tz::window().get_width() / tz::window().get_height();
}

int main()
{
    tz::WindowInitArgs wargs = tz::default_args;
    wargs.resizeable = false;
    tz::initialise({"tz_compute_demo", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::WindowApplication, wargs);
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        tz::gl::ShaderBuilder shader_builder;
        shader_builder.set_shader_file(tz::gl::ShaderType::ComputeShader, ".\\demo\\gl\\compute_demo.compute.tzsl");

        tz::gl::Shader shader = device.create_shader(shader_builder);

        tz::gl::ProcessorBuilder processor_builder;
        tz::gl::BufferResource compute_buf{tz::gl::BufferData::from_array<tz::Mat4>
        ({{
            tz::model({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}),
            tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}),
            tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f)
        }})};
        processor_builder.add_resource(compute_buf);
        processor_builder.set_shader(shader);
        tz::gl::Processor processor = device.create_processor(processor_builder);

        // Render something placeholder to the screen. Not necessary but otherwise may yield output differences between VK/OGL versions
        tz::gl::ShaderBuilder render_shader_builder;
        render_shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\demo\\gl\\triangle_demo.vertex.tzsl");
        render_shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\demo\\gl\\triangle_demo.fragment.tzsl");

        tz::gl::Shader render_shader = device.create_shader(render_shader_builder);

        tz::gl::RendererBuilder renderer_builder;
        tz::gl::Mesh mesh;
        mesh.vertices =
            {
                tz::gl::Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {}, {}, {}}
            };
        mesh.indices = {0, 1, 2};
        tz::gl::MeshInput mesh_input{mesh};
        // Note: Window is resizeable but we don't amend the aspect-ratio if it does. This is for simplicity's sake -- This is done properly in tz_dynamic_triangle_demo.
        tz::gl::BufferResource buf_res{tz::gl::BufferData::from_array<tz::Mat4>
        ({{
            tz::model({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}),
            tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}),
            tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f)
        }})};

        renderer_builder.add_input(mesh_input);
        renderer_builder.set_output(tz::window());
        renderer_builder.add_resource(buf_res);
        renderer_builder.set_shader(render_shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        renderer.set_clear_colour({0.1f, 0.2f, 0.4f, 1.0f});

        while(!tz::window().is_close_requested())
        {
            tz::window().update();
            processor.process();
            renderer.render();
        }
    }
    tz::terminate();
}