#ifndef TOPAZ_WSI_MOUSE_HPP
#define TOPAZ_WSI_MOUSE_HPP
#include "tz/wsi/api/mouse.hpp"

namespace tz::wsi
{
	bool is_mouse_button_down(const tz::wsi::mouse_state& ms, tz::wsi::mouse_button b);
}

#endif // TOPAZ_WSI_MOUSE_HPP
