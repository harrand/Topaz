#include "core/tz.hpp"
#include "gl/2/device.hpp"
#include "gl/2/renderer.hpp"
#include "gl/2/resource.hpp"

int main()
{
	tz::GameInfo g{"tz_triangle_demo (gl2)", {1, 0, 0}, tz::info()};
	tz::initialise(g, tz::ApplicationType::WindowApplication);
	tz::gl::vk2::initialise(g, tz::ApplicationType::WindowApplication);
	{
		tz::gl2::Device dev;

		tz::gl2::ImageResource img = tz::gl2::ImageResource::from_uninitialised(tz::gl2::ImageFormat::RGBA32, {64u, 64u});

		tz::gl2::RendererInfo rinfo;
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
