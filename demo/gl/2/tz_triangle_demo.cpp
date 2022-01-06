#include "core/tz.hpp"
#include "gl/2/device.hpp"

int main()
{
	tz::GameInfo g{"tz_triangle_demo (gl2)", {1, 0, 0}, tz::info()};
	tz::initialise(g, tz::ApplicationType::WindowApplication);
	tz::gl::vk2::initialise(g, tz::ApplicationType::WindowApplication);
	{
		tz::gl2::Device dev;

		while(!tz::window().is_close_requested())
		{
			tz::window().update();
		}
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
