#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/component.hpp"
#include "gl/input.hpp"
#include "gl/output.hpp"
#include "gl/shader.hpp"

float get_aspect_ratio()
{
    return tz::window().get_width() / tz::window().get_height();
}

int main()
{
    tz::initialise({"tz_invert_demo", tz::Version{1, 0, 0}, tz::info()});
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        // Firstly, setup the renderer which simply inverts the image resource it is provided.
        tz::gl::ShaderBuilder invert_shader_builder;
        invert_shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\demo\\gl\\invert_triangle_demo.vertex.tzsl");
        invert_shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\demo\\gl\\invert_triangle_demo.fragment.tzsl");

        float w = tz::window().get_width();
        float h = tz::window().get_height();
        tz::gl::TextureFormat wnd_fmt = device.get_window_format();
        tz::gl::TextureResource tex_res{tz::gl::TextureData::uninitialised(w, h, wnd_fmt), wnd_fmt};

        tz::gl::Shader invert_shader = device.create_shader(invert_shader_builder);

        tz::gl::RendererBuilder invert_renderer_builder;
        tz::gl::ResourceHandle render_target_handle = invert_renderer_builder.add_resource(tex_res);
        invert_renderer_builder.set_output(tz::window());
        invert_renderer_builder.set_shader(invert_shader);

        tz::gl::Renderer invert_renderer = device.create_renderer(invert_renderer_builder);
        invert_renderer.set_clear_colour({1.0f, 1.0f, 0.0f, 1.0f});

        auto* render_target_component = static_cast<tz::gl::TextureComponent*>(invert_renderer.get_component(render_target_handle));
        tz::gl::TextureOutput render_target;
        render_target.add_colour_output(render_target_component);

        // Secondly, setup the renderer which actually draws the triangle.

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
        renderer_builder.set_output(render_target);
        renderer_builder.add_resource(buf_res);
        renderer_builder.set_pass(tz::gl::RenderPassAttachment::Colour);
        renderer_builder.set_shader(shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        renderer.set_clear_colour({0.1f, 0.2f, 0.4f, 1.0f});

        auto draw_scene = [&renderer, &invert_renderer]()
        {
            renderer.render();
            invert_renderer.render();
        };

        tz::window().add_resize_callback([&draw_scene]([[maybe_unused]] int w, [[maybe_unused]] int h)
        {
            draw_scene();
        });

        while(!tz::window().is_close_requested())
        {
            tz::window().update();
            draw_scene();
        }
    }
    tz::terminate();
}