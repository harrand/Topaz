#ifndef TZ_WSI_API_MOUSE_HPP
#define TZ_WSI_API_MOUSE_HPP
#include "tz/core/data/vector.hpp"
#include <array>

namespace tz::wsi
{
	/**
	 * @ingroup tz_wsi_mouse
	 * Represents all supported mouse buttons.
	 */ 
	enum class mouse_button
	{
		/// Left mouse button.
		left,
		/// Right mouse button.
		right,
		/// Middle mouse button.
		middle,
		_count
	};

	/**
	 * @ingroup tz_wsi_mouse
	 * Represents the current state of a @ref mouse_button
	 */
	enum class mouse_button_state
	{
		/// Mouse button has been clicked.
		clicked,
		/// Mouse button has been double-clicked.
		double_clicked,
		/// Mouse button is not pressed.
		noclicked,
	};

	/**
	 * @ingroup tz_wsi_mouse
	 * Represents the total state of the mouse for a single window. Retrieve via @ref tz::wsi::window::get_mouse_state()
	 */
	struct mouse_state
	{
		/// Array of mouse button states against their booleans. Access via `button_state[(int)mouse_button::left].`
		std::array<mouse_button_state, static_cast<int>(mouse_button::_count)> button_state;
		/// Current position of the mouse cursor within the window.
		tz::vec2ui mouse_position;
		/// Current elevation of the mouse wheel. Increments per scroll-up, decrements per scroll-down.
		int wheel_position = 0;
	};
}

#endif // TZ_WSI_API_MOUSE_HPP
