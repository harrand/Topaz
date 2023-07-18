#include "tz/wsi/mouse.hpp"
#include "tz/tz.hpp"
#include "tz/core/debug.hpp"

int main()
{
	tz::initialise
	({
		.name = "tz_mouse_press_demo",
		.flags =
		{
			tz::application_flag::no_graphics,
		  	tz::application_flag::no_dbgui
		}
	});
	{
		const auto& ms = tz::window().get_mouse_state();
		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			for(int i = 0; i < static_cast<int>(tz::wsi::mouse_button::_count); i++)
			{
				auto but = static_cast<tz::wsi::mouse_button>(i);
				if(tz::wsi::is_mouse_button_down(ms, but))
				{
					std::printf("mouse button %d\n", i);
				}
			}
			std::printf("\n");
			tz::end_frame();
		}
	}
	tz::terminate();
}
