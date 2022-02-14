#include "core/peripherals/mouse.hpp"
#include "core/tz.hpp"
#include "core/report.hpp"

int main()
{
	tz::initialise({"tz_mouse_press_demo", tz::Version{1, 0, 0}});
	{
		while(!tz::window().is_close_requested())
		{
			const tz::MouseButtonState& mb = tz::window().get_mouse_button_state();
			constexpr tz::MouseButtonInfo left = tz::peripherals::mouse::get_mouse_button(tz::MouseButton::Left);
			std::printf("Left click is %spressed                 \r", mb.is_mouse_button_down(left) ? "" : "not ");
			tz::window().update();
		}
		std::printf("\n");
	}
	tz::terminate();
}
