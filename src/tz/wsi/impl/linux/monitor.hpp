#ifndef TANGLE_IMPL_LINUX_MONITOR_HPP
#define TANGLE_IMPL_LINUX_MONITOR_HPP 
#ifdef __linux__
#include "tz/wsi/api/monitor.hpp"
#include <vector>

namespace tz::wsi::impl
{
	std::vector<tz::wsi::monitor> get_monitors_linux();
}

#endif // __linux__
#endif // TANGLE_IMPL_LINUX_MONITOR_HPP
