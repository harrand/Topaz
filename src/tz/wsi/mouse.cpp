#include "tz/wsi/mouse.hpp"
#include <algorithm>

namespace tz::wsi
{
	bool is_mouse_button_down(const tz::wsi::mouse_state& ms, tz::wsi::mouse_button b)
	{
		return ms.button_state[static_cast<int>(b)] != tz::wsi::mouse_button_state::noclicked;
	}
}
