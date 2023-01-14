#include "tz/wsi/monitor.hpp"
#include "tz/wsi/impl/windows/monitor.hpp"
#include "tz/wsi/impl/linux/monitor.hpp"

namespace tz::wsi
{
	std::vector<tz::wsi::monitor> get_monitors()
	{
		#ifdef _WIN32
			return impl::get_monitors_windows();
		#elif defined(__linux__)
			return impl::get_monitors_linux();
		#else
			static_assert(false);
			return {};
		#endif
	}
}
