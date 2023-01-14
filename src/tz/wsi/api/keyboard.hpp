#ifndef TANGLE_API_KEYBOARD_HPP
#define TANGLE_API_KEYBOARD_HPP
#include <array>

namespace tz::wsi
{
	constexpr unsigned int max_simultaneous_key_presses = 64u;
	enum class key
	{
		esc,
		f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, del,
		page_up, page_down, print_screen, scroll_lock, pause, insert,
		one, two, three, four, five, six, seven, eight, nine, zero, minus, equals, backspace, tab, q, w, e, r, t, y, u, i, o, p, left_bracket, right_bracket, enter, caps_lock, a, s, d, f, g, h, j, k, l, semi_colon, apostrophe, hash, left_shift, backslash, z, x, c, v, b, n, m, comma, period, forward_slash, right_shift, left_ctrl, win_key, alt, space, alt_gr, right_ctrl,
		unknown
	};
	struct keyboard_state
	{
		std::array<key, max_simultaneous_key_presses> keys_down;
		mutable key last_key = key::unknown;

		key pop_last_key() const{key k = this->last_key; this->last_key = key::unknown; return k;}
	};
}

#endif // TANGLE_API_KEYBOARD_HPP
