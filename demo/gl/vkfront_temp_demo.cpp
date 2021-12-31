#include "core/tz.hpp"
#include "gl/mesh.hpp"
#include "gl/input.hpp"
#include "gl/resource.hpp"
#include "gl/texture.hpp"
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

		tz::gl::ShaderBuilderVulkan2 shader_builder;
		shader_builder.set_shader_file(tz::gl::ShaderType::VertexShader, "./demo/gl/vkfront_temp_demo.vertex.tzsl");
		shader_builder.set_shader_file(tz::gl::ShaderType::FragmentShader, "./demo/gl/vkfront_temp_demo.fragment.tzsl");
		tz::gl::ShaderVulkan2 shader = dev.create_shader(shader_builder);

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

		tz::gl::RendererBuilderVulkan2 renderer_builder;
		renderer_builder.set_shader(shader);
		renderer_builder.add_resource(texture);
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
