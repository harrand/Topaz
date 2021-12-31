#include "core/tz.hpp"
#include "gl/mesh.hpp"
#include "gl/input.hpp"
#include "gl/impl/frontend/vk2/device.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"

int main()
{
	tz::GameInfo g{"vk2front_temp_demo", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(g, tz::ApplicationType::WindowApplication,
	{
		.width = 800,
		.height = 600,
		.resizeable = false
	});
	tz::gl::vk2::initialise(g, tz::ApplicationType::WindowApplication);
	{
		tz::gl::DeviceVulkan dev;

		tz::gl::Mesh mesh;
		mesh.vertices =
			{
				tz::gl::Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {}, {}, {}},
				tz::gl::Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {}, {}, {}},
				tz::gl::Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {}, {}, {}}
			};
		mesh.indices = {0, 1, 2};
		tz::gl::MeshInput mesh_input{mesh};

		tz::gl::RendererBuilderVulkan2 renderer_builder;
		renderer_builder.add_input(mesh_input);
		renderer_builder.set_output(tz::window());

		tz::gl::RendererVulkan2 renderer = dev.create_renderer(renderer_builder);

		int x = 5;
		while(!tz::window().is_close_requested())
		{
			tz::window().update();
		}
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
