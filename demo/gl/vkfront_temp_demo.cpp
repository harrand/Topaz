#include "core/tz.hpp"
#include "gl/impl/frontend/vk2/device.hpp"

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
		int x = 5;
		while(tz::window().is_close_requested())
		{
			tz::window().update();
		}
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
