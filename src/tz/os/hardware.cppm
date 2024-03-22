module;
export module topaz.os:hardware;

export import :hardware_common;
#ifdef _WIN32
import :impl_windows;
#endif

import topaz.debug;

export namespace tz::os
{
	system get_system()
	{
		#ifdef _WIN32
			return windows::get_system();
		#else
			tz::debug::error("get_system() is not yet implemented on your platform.");	
		#endif
		return {};
	}
}
