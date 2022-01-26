#include "core/tz.hpp"
#include "gl/2/device.hpp"
#include "gl/2/renderer.hpp"
#include "gl/2/resource.hpp"
#include "gl/imported_shaders.hpp"

#include ImportedShaderFile(tz_triangle_demo.vertex)
#include ImportedShaderFile(tz_triangle_demo.fragment)

int main()
{
	tz::GameInfo g{"tz_triangle_demo (gl2)", {1, 0, 0}, tz::info()};
	tz::initialise(g, tz::ApplicationType::WindowApplication);
	tz::gl::vk2::initialise(g, tz::ApplicationType::WindowApplication);
	{
		tz::gl2::Device dev;

		// TODOOO: Don't use uninitialised data. During testing it appeared completely black, so don't yet see anything!
		std::vector<unsigned char> imgdata = 
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
		}};
		tz::gl2::ImageResource img = tz::gl2::ImageResource::from_memory(tz::gl2::ImageFormat::RGBA32, {2u, 2u}, std::as_bytes(static_cast<std::span<unsigned char>>(imgdata)));

		tz::gl2::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl2::ShaderStage::Vertex, GetImportedShaderSource(tz_triangle_demo_vertex));
		rinfo.shader().set_shader(tz::gl2::ShaderStage::Fragment, GetImportedShaderSource(tz_triangle_demo_fragment));
		rinfo.add_resource(img);

		tz::gl2::Renderer renderer = dev.create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			tz::window().update();
			renderer.render();
		}
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
