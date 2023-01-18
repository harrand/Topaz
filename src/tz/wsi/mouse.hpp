#ifndef TOPAZ_WSI_MOUSE_HPP
#define TOPAZ_WSI_MOUSE_HPP
#include "tz/wsi/api/mouse.hpp"

namespace tz::wsi
{
	/**
	 * @ingroup tz_wsi_mouse
	 * Query as to whether a specific mouse button is currently pressed for a mouse state (clicked or double-clicked).
	 * @param ms Mouse state to query.
	 * @param b Which mouse button are you asking to see is pressed?
	 * @return True if `b` is pressed according to the passed mouse state, false if not.
	 */
	bool is_mouse_button_down(const tz::wsi::mouse_state& ms, tz::wsi::mouse_button b);
}

#endif // TOPAZ_WSI_MOUSE_HPP
