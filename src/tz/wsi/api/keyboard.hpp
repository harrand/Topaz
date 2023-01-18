#ifndef TZ_WSI_API_KEYBOARD_HPP
#define TZ_WSI_API_KEYBOARD_HPP
#include <array>

namespace tz::wsi
{
	constexpr unsigned int max_simultaneous_key_presses = 64u;
	/**
	 * @ingroup tz_wsi_keyboard
	 * Contains all possible keyboard inputs that Topaz supports.
	 */
	enum class key
	{
		esc,
		f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, del,
		page_up, page_down, print_screen, scroll_lock, pause, insert,
		one, two, three, four, five, six, seven, eight, nine, zero, minus, equals, backspace, tab, q, w, e, r, t, y, u, i, o, p, left_bracket, right_bracket, enter, caps_lock, a, s, d, f, g, h, j, k, l, semi_colon, apostrophe, hash, left_shift, backslash, z, x, c, v, b, n, m, comma, period, forward_slash, right_shift, left_ctrl, win_key, alt, space, alt_gr, right_ctrl,
		unknown
	};

	/**
	 * @ingroup tz_wsi_keyboard
	 * Represents the total state of the keyboard and key-presses for a single window. Retrieve via @ref tz::wsi::window::get_keyboard_state();
	 */
	struct keyboard_state
	{
		/// List of keys currently pressed. There is an implementation-defined maximum number of keys that can be pressed simultaneously (at present, 64).
		std::array<key, max_simultaneous_key_presses> keys_down;
		/// The last key pressed, or @ref tz::wsi::key::unknown if none.
		mutable key last_key = key::unknown;

		/**
		 * Retrieve the last pressed key, and reset that state to `key::unknown`. Returns @ref tz::wsi::key::unknown if either no key has ever been pressed, or the last key state has already been reset.
		 */
		key pop_last_key() const{key k = this->last_key; this->last_key = key::unknown; return k;}
	};
}

#endif // TZ_WSI_API_KEYBOARD_HPP
