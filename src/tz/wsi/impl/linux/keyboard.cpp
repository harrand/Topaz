#ifdef __linux__
#include "tz/wsi/impl/linux/keyboard.hpp"

namespace tz::wsi::impl
{
	std::string get_key_name_linux(tz::wsi::key key)
	{
		return "NYI";
	}

	std::string get_chars_typed_linux(tz::wsi::key key, const keyboard_state& state)
	{
		return "NYI";
	}
}

#endif // __linux__
