#ifndef TANGLE_IMPL_WINDOWS_KEYBOARD_HPP
#define TANGLE_IMPL_WINDOWS_KEYBOARD_HPP
#ifdef _WIN32
#include "tz/wsi/api/keyboard.hpp"
#include <string>

namespace tz::wsi::impl
{
	constexpr tz::wsi::key win_to_tge_key(int virtual_key_code);
	constexpr int tge_to_win_key(tz::wsi::key key);
	std::string get_key_name_windows(tz::wsi::key key);
	std::string get_chars_typed_windows(tz::wsi::key key, const keyboard_state& state);
}

#include "tz/wsi/impl/windows/keyboard.inl"

#endif // _WIN32
#endif // TANGLE_IMPL_WINDOWS_KEYBOARD_HPP
