#ifndef TOPAZ_OS_INPUT_HPP
#define TOPAZ_OS_INPUT_HPP
#include "tz/core/error.hpp"

namespace tz::os
{
	/**
	 * @ingroup tz_os
	 * @defgroup tz_os_input Input System
	 * @brief Retrieve keyboard/mouse input.
	 **/

	/**
	 * @ingroup tz_os_input
	 * Represents a key on your keyboard.
	 **/
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

	using char_type_callback = void(*)(char);
	/**
	 * @ingroup tz_os_input
	 * @brief Install a callback which will be invoked whenever the user types a character on the keyboard.
	 * @param callback Pointer to a function which will be called whenever a character is typed. If you pass nullptr, the callback will safely be ignored.
	 * @return - @ref error_code::precondition_failure if a window has not yet been opened.
	 **/
	tz::error_code install_char_typed_callback(char_type_callback callback);
}

#endif // TOPAZ_OS_INPUT_HPP