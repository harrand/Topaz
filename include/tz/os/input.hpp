#ifndef TOPAZ_OS_INPUT_HPP
#define TOPAZ_OS_INPUT_HPP
#include "tz/core/error.hpp"

namespace tz::os
{
	enum class key
	{
		// Letters
		a, b, c, d, e, f, g,
		h, i, j, k, l, m, n,
		o, p, q, r, s, t, u,
		v, w, x, y, z,

		// Numbers
		num_0, num_1, num_2, num_3, num_4,
		num_5, num_6, num_7, num_8, num_9,
		undefined
	};

	enum class key_state
	{
		pressed,
		released
	};
	using char_type_callback = void(*)(char);
	/// Returns @ref precondition_failure if a window has not yet been opened.
	tz::error_code install_char_typed_callback(char_type_callback callback);
}

#endif // TOPAZ_OS_INPUT_HPP