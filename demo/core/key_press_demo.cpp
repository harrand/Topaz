#include "core/peripherals/keyboard.hpp"
#include "core/tz.hpp"
#include "core/report.hpp"

int main()
{
	tz::initialise({"tz_key_press_demo", tz::Version{1, 0, 0}});
	{
		while(!tz::window().is_close_requested())
		{
			const tz::KeyboardState& kb = tz::window().get_keyboard_state();
			tz::KeyInfo w = tz::detail::peripherals::keyboard::get_key('W');
			std::printf("W is %spressed, %srepeating                 \r", kb.is_key_down(w) ? "" : "not ", kb.is_key_repeating(w) ? "" : "not ");
			tz::window().update();
		}
		std::printf("\n");
	}
	tz::terminate();
}
