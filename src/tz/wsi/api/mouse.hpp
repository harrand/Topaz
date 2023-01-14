#ifndef TZ_WSI_API_MOUSE_HPP
#define TZ_WSI_API_MOUSE_HPP
#include "hdk/data/vector.hpp"
#include <array>

namespace tz::wsi
{
	enum class mouse_button
	{
		left, right, middle,

		_count
	};

	enum class mouse_button_state
	{
		clicked,
		double_clicked,
		noclicked,
	};

	struct mouse_state
	{
		std::array<mouse_button_state, static_cast<int>(mouse_button::_count)> button_state;
		hdk::vec2ui mouse_position;
		int wheel_position = 0;
	};
}

#endif // TZ_WSI_API_MOUSE_HPP
