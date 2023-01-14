#ifndef TANGLE_IMPL_LINUX_KEYBOARD_HPP
#ifdef __linux__
#include "tz/wsi/api/keyboard.hpp"
#include <string>

namespace tz::wsi::impl
{
	std::string get_key_name_linux(tz::wsi::key key);
	std::string get_chars_typed_linux(tz::wsi::key key, const keyboard_state& state);
}

#endif // __linux__
#endif // TANGLE_IMPL_LINUX_KEYBOARD_HPP
