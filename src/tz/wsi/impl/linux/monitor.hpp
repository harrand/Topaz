#ifndef TZ_WSI_IMPL_LINUX_MONITOR_HPP
#define TZ_WSI_IMPL_LINUX_MONITOR_HPP 
#ifdef __linux__
#include "tz/wsi/api/monitor.hpp"
#include <vector>

namespace tz::wsi::impl
{
	std::vector<tz::wsi::monitor> get_monitors_linux();
}

#endif // __linux__
#endif // TZ_WSI_IMPL_LINUX_MONITOR_HPP
