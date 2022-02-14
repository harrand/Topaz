#include "core/peripherals/mouse.hpp"
#include "core/tz.hpp"
#include "core/report.hpp"

int main()
{
	tz::initialise({"tz_mouse_press_demo", tz::Version{1, 0, 0}});
	{
		while(!tz::window().is_close_requested())
		{
			tz::window().get_mouse_button_state().debug_print_state();
			tz::window().update();
		}
		std::printf("\n");
	}
	tz::terminate();
}
