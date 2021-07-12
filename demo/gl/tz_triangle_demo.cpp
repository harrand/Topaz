#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/input.hpp"
#include "gl/shader.hpp"
#include "Tracy.hpp"

float get_aspect_ratio()
{
    return tz::window().get_width() / tz::window().get_height();
}

int main()
{
    tz::initialise({"tz_triangle_demo", tz::Version{1, 0, 0}, tz::info()});
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        tz::gl::RenderPassBuilder pass_builder;
        pass_builder.add_pass(tz::gl::RenderPassAttachment::Colour);
        tz::gl::RenderPass render_pass = device.create_render_pass(pass_builder);

        tz::gl::ShaderBuilder shader_builder;
        shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\demo\\gl\\triangle_demo.vertex.tzsl");
        shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\demo\\gl\\triangle_demo.fragment.tzsl");

        tz::gl::Shader shader = device.create_shader(shader_builder);

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
        renderer_builder.set_render_pass(render_pass);
        renderer_builder.set_shader(shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        renderer.set_clear_colour({0.1f, 0.2f, 0.4f, 1.0f});
        while(!tz::window().is_close_requested())
        {
            #if TZ_PROFILE
                constexpr char const* tracy_label = "WindowCloseRequestedLoop";
                FrameMarkStart(tracy_label);
            #endif
            tz::window().update();
            renderer.render();
            #if TZ_PROFILE
                FrameMarkEnd(tracy_label);
            #endif
        }
    }
    tz::terminate();
}