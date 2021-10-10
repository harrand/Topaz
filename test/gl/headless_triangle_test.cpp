#include "core/tz.hpp"
#include "core/vector.hpp"
#include "core/matrix_transform.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/input.hpp"
#include "gl/shader.hpp"

int main()
{
	tz::initialise({"tz_headless_triangle_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
	{
		tz::gl::DeviceBuilder device_builder;
		tz::gl::Device device{device_builder};

		tz::gl::ShaderBuilder shader_builder;
		shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\triangle_test.vertex.tzsl");
		shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\triangle_test.fragment.tzsl");

		tz::gl::Shader shader = device.create_shader(shader_builder);

		tz::gl::RendererBuilder renderer_builder;
		tz::gl::Mesh mesh;
		mesh.vertices =
		{
				tz::gl::Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {}, {}, {}},
				tz::gl::Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {}, {}, {}},
				tz::gl::Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {}, {}, {}}
		};
		mesh.indices = { 0, 1, 2 };
		tz::gl::MeshInput mesh_input{mesh};

		renderer_builder.add_input(mesh_input);
		renderer_builder.set_output(tz::window());
		renderer_builder.set_shader(shader);
		tz::gl::Renderer renderer = device.create_renderer(renderer_builder);
		renderer.set_clear_colour({0.1f, 0.2f, 0.4f, 1.0f});

		// Just render 64 frames.
		for(std::size_t i = 0; i < 64; i++)
		{
			renderer.render();
		}
	}
	tz::terminate();
}