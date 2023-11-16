#include "tz/tz.hpp"
#include "tz/gl/device.hpp"

int main()
{
	tz::initialise({.name = "tz_text_rendering_demo"});
	{
		tz::begin_frame();
		tz::gl::get_device().render();
		tz::end_frame();
	}
	tz::terminate();
}