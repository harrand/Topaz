#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/mesh.hpp"
#include "gl/shader.hpp"
#include "gl/texture.hpp"
#include "gl/vk/tz_vulkan.hpp"

float get_aspect_ratio()
{
    return tz::window().get_width() / tz::window().get_height();
}

int main()
{
    tz::initialise({"tz_multi_draw_demo", tz::Version{1, 0, 0}, tz::info()});
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        tz::gl::RenderPassBuilder pass_builder;
        pass_builder.add_pass(tz::gl::RenderPassAttachment::ColourDepth);
        tz::gl::RenderPass render_pass = device.create_render_pass(pass_builder);

        tz::gl::ShaderBuilder shader_builder;
        shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\demo\\gl\\dynamic_triangle_demo.vertex.tzsl");
        shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\demo\\gl\\dynamic_triangle_demo.fragment.tzsl");

        tz::gl::Shader shader = device.create_shader(shader_builder);

        tz::gl::RendererBuilder renderer_builder;
        tz::gl::MeshInput mesh_input{tz::gl::Mesh
        {
            .vertices =
            {
                tz::gl::Vertex{{-0.5f, -0.5f, -0.2f}, {1.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.5f, -0.5f, -0.2f}, {0.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.5f, 0.5f, -0.2f}, {0.0f, 1.0f}, {}, {}, {}}
            },
            .indices = {0, 1, 2}
        }};

        tz::gl::MeshInput mesh_input2{tz::gl::Mesh
        {
            .vertices =
            {
                tz::gl::Vertex{{-0.25f, -0.25f, -0.1f}, {1.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.25f, -0.25f, -0.1f}, {0.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.25f, 0.25f, -0.1f}, {0.0f, 1.0f}, {}, {}, {}},
                tz::gl::Vertex{{-0.25f, 0.25f, -0.1f}, {1.0f, 1.0f}, {}, {}, {}},
            },
            .indices = {0, 1, 2, 2, 3, 0}
        }};

        tz::gl::MeshDynamicInput mesh_input3{tz::gl::Mesh
        {
            .vertices =
            {
                tz::gl::Vertex{{-0.125f, -0.125f, 0.0f}, {1.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.125f, -0.125f, 0.0f}, {0.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.125f, 0.125f, 0.0f}, {0.0f, 1.0f}, {}, {}, {}},
                tz::gl::Vertex{{-0.125f, 0.125f, 0.0f}, {1.0f, 1.0f}, {}, {}, {}},
            },
            .indices = {0, 1, 2, 2, 3, 0}
        }};

        tz::gl::TextureResource texture{tz::gl::TextureData::FromMemory(2, 2,
        {{
            0b0000'0000,
            0b0000'0000,
            0b1111'1111,
            0b1111'1111,

            0b1111'1111,
            0b0000'0000,
            0b0000'0000,
            0b1111'1111,

            0b0000'0000,
            0b1111'1111,
            0b0000'0000,
            0b1111'1111,

            0b0000'0000,
            0b0000'0000,
            0b1111'1111,
            0b1111'1111
        }}), tz::gl::TextureFormat::Rgba32sRGB};

        tz::gl::DynamicBufferResource buf_res{tz::gl::BufferData::FromArray<tz::Mat4>
        ({{
            tz::model({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}),
            tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}),
            tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f)
        }})};

        tz::gl::RendererInputHandle handle1 = renderer_builder.add_input(mesh_input);
        tz::gl::RendererInputHandle handle2 = renderer_builder.add_input(mesh_input2);
        tz::gl::RendererInputHandle handle3 = renderer_builder.add_input(mesh_input3);
        renderer_builder.set_output(tz::window());
        tz::gl::ResourceHandle buf_handle = renderer_builder.add_resource(buf_res);
        renderer_builder.add_resource(texture);
        renderer_builder.set_render_pass(render_pass);
        renderer_builder.set_shader(shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        renderer.set_clear_colour({0.1f, 0.2f, 0.4f, 1.0f});

        tz::gl::RendererDrawList draws{handle2, handle3};

        while(!tz::window().is_close_requested())
        {
            static float counter = 0.0f;
            // Every frame, update some of the buffer resource data.
            {
                auto buffer_bytes = static_cast<tz::gl::IDynamicResource*>(renderer.get_resource(buf_handle))->get_resource_bytes_dynamic();
                // Change the position of the triangle ever so slightly.
                tz::Mat4& model = reinterpret_cast<tz::Mat4*>(buffer_bytes.data())[0];
                model = tz::model({std::sin(counter * 0.25f) * 0.5f * get_aspect_ratio(), std::sin(counter) * 0.25f, -1.0f}, {0.0f, 0.0f, std::sin(counter * 0.1f) * 3.14159f}, {1.0f, 1.0f, 1.0f});
                counter += 0.001f;

                // Then update aspect ratio of mvp perspective.
                tz::Mat4& projection = reinterpret_cast<tz::Mat4*>(buffer_bytes.data())[2];
                projection = tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f);
            }

            {
                // Also, mess around with the smaller square (which is a dynamic input)
                auto* input3 = static_cast<tz::gl::IRendererDynamicInput*>(renderer.get_input(handle3));
                auto* input3_vertices = reinterpret_cast<tz::gl::Vertex*>(input3->get_vertex_bytes_dynamic().data());
                for(std::size_t i = 0; i < 4; i++)
                {
                    input3_vertices[i].position[2] = std::sin(counter) * 0.1f;
                }
            }

            tz::window().update();
            renderer.render(draws);
        }
    }
    tz::terminate();
}