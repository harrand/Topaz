#include "tz/core/peripherals/keyboard.hpp"
#include "tz/tz.hpp"
#include "tz/core/debug.hpp"

int main()
{
	tz::initialise({"tz_key_press_demo", tz::version{1, 0, 0}});
	{
		while(!tz::window().is_close_requested())
		{
			tz::window().get_keyboard_state().debug_print_state();
			tz::window().update();
		}
		std::printf("\n");
	}
	tz::terminate();
}
