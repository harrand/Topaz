#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/input.hpp"
#include "gl/shader.hpp"
#include "gl/texture.hpp"

float get_aspect_ratio()
{
    return tz::window().get_width() / tz::window().get_height();
}

int main()
{
    tz::initialise({"tz_dynamic_triangle_demo", tz::Version{1, 0, 0}, tz::info()});
    {
        tz::gl::DeviceBuilder device_builder;
        tz::gl::Device device{device_builder};

        tz::gl::ShaderBuilder shader_builder;
        shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\demo\\gl\\dynamic_triangle_demo.vertex.tzsl");
        shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\demo\\gl\\dynamic_triangle_demo.fragment.tzsl");

        tz::gl::Shader shader = device.create_shader(shader_builder);

        tz::gl::RendererBuilder renderer_builder;
        tz::gl::MeshInput mesh_input{tz::gl::Mesh
        {
            .vertices =
            {
                tz::gl::Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {}, {}, {}},
                tz::gl::Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {}, {}, {}}
            },
            .indices = {0, 1, 2}
        }};

        tz::gl::TextureResource texture{tz::gl::TextureData::from_memory(2, 2,
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

        tz::gl::DynamicBufferResource buf_res{tz::gl::BufferData::from_array<tz::Mat4>
        ({{
            tz::model({0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}),
            tz::view({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}),
            tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f)
        }})};

        renderer_builder.add_input(mesh_input);
        renderer_builder.set_output(tz::window());
        tz::gl::ResourceHandle buf_handle = renderer_builder.add_resource(buf_res);
        renderer_builder.add_resource(texture);
        renderer_builder.set_shader(shader);
        tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
        renderer.set_clear_colour({0.1f, 0.2f, 0.4f, 1.0f});
        while(!tz::window().is_close_requested())
        {
            // Every frame, update some of the buffer resource data.
            {
                auto buffer_bytes = static_cast<tz::gl::IDynamicResource*>(renderer.get_resource(buf_handle))->get_resource_bytes_dynamic();
                // Change the position of the triangle ever so slightly.
                tz::Mat4& model = reinterpret_cast<tz::Mat4*>(buffer_bytes.data())[0];
                static float counter = 0.0f;
                model = tz::model({std::sin(counter * 0.25f) * 0.5f * get_aspect_ratio(), std::sin(counter) * 0.25f, -1.0f}, {0.0f, 0.0f, std::sin(counter * 0.1f) * 3.14159f}, {1.0f, 1.0f, 1.0f});
                counter += 0.001f;

                // Then update aspect ratio of mvp perspective.
                tz::Mat4& projection = reinterpret_cast<tz::Mat4*>(buffer_bytes.data())[2];
                projection = tz::perspective(1.27f, get_aspect_ratio(), 0.1f, 1000.0f);
            }

            tz::window().update();
            renderer.render();
        }
    }
    tz::terminate();
}