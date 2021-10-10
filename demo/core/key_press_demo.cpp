#include "core/tz.hpp"
#include "core/report.hpp"

int main()
{
	tz::initialise({"tz_key_press_demo", tz::Version{1, 0, 0}});
	{
		while(!tz::window().is_close_requested())
		{
			std::printf("W is %spressed          \r", tz::window().is_key_pressed(GLFW_KEY_W) ? "" : "not ");
			tz::window().update();
		}
	}
	tz::terminate();
}