#ifndef TZ_WSI_KEYBOARD_HPP
#define TZ_WSI_KEYBOARD_HPP
#include "tz/wsi/api/keyboard.hpp"
#include <string>

namespace tz::wsi
{
	std::string get_key_name(tz::wsi::key key);
	std::string get_chars_typed(tz::wsi::key key, const keyboard_state& state);
}

#endif // TZ_WSI_KEYBOARD_HPP
