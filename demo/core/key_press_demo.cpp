#include "tz/wsi/keyboard.hpp"
#include "tz/tz.hpp"
#include "tz/core/debug.hpp"

int main()
{
	tz::initialise
	({
		.name = "tz_key_press_demo",
		.flags =
		{
			tz::application_flag::no_graphics,
		  	tz::application_flag::no_dbgui
		}
	});
	{
		const tz::wsi::keyboard_state& ks = tz::window().get_keyboard_state();
		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			for(int i = 0; i < static_cast<int>(tz::wsi::key::unknown); i++)
			{
				auto key = static_cast<tz::wsi::key>(i);
				if(tz::wsi::is_key_down(ks, key))
				{
					std::printf("%s", tz::wsi::get_key_name(key).c_str());
				}
			}
			std::printf("\n");
			tz::end_frame();
		}
	}
	tz::terminate();
}
