#ifndef TOPAZ_OS_WINDOW_HPP
#define TOPAZ_OS_WINDOW_HPP
#include "tz/core/handle.hpp"
#include <string>

namespace tz::os
{
	enum window_flags
	{
		none = 0x00,
		/// Ignore the x and y coordinates provided within @ref window_info and position the window exactly at the centre of the screen instead.
		centered_window = 0x01,
	};

	constexpr window_flags operator|(window_flags lhs, window_flags rhs)
	{
		return static_cast<window_flags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	struct window_info
	{
		std::string name = "Untitled";
		unsigned int x = 0;
		unsigned int y = 0;
		unsigned int width = 800;
		unsigned int height = 600;
		window_flags flags = window_flags::none;
	};

	using window_handle = tz::handle<window_info>;

	window_handle create_window(window_info winfo);
}

#endif // TOPAZ_OS_WINDOW_HPP