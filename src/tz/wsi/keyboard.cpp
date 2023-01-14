#include "tz/wsi/keyboard.hpp"
#include "tz/wsi/impl/linux/keyboard.hpp"
#include "tz/wsi/impl/windows/keyboard.hpp"

namespace tz::wsi
{
	std::string get_key_name(tz::wsi::key key)
	{
		#ifdef _WIN32
			return tz::wsi::impl::get_key_name_windows(key);
		#elif defined(__linux__)
			return tz::wsi::impl::get_key_name_linux(key);
		#else
			static_assert(false, "tz::wsi::get_key_name not implemented for target platform.");
			return "";
		#endif
	}

	std::string get_chars_typed(tz::wsi::key key, const keyboard_state& state)
	{
		#ifdef _WIN32
			return tz::wsi::impl::get_chars_typed_windows(key, state);
		#elif defined(__linux__)
			return tz::wsi::impl::get_chars_typed_linux(key, state);
		#else
			static_assert(false, "tz::wsi::get_chars_typed not implemented for target platform.");
			return "";
		#endif
	}
}
