#ifndef TZ_WSI_IMPL_WINDOWS_MONITOR_HPP
#define TZ_WSI_IMPL_WINDOWS_MONITOR_HPP
#ifdef _WIN32
#include "tz/wsi/api/monitor.hpp"

namespace tz::wsi::impl
{
	std::vector<tz::wsi::monitor> get_monitors_windows();
}

#endif // _WIN32
#endif // TZ_WSI_IMPL_WINDOWS_MONITOR_HPP
