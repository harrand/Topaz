#ifndef TANGLE_MONITOR_HPP
#define TANGLE_MONITOR_HPP
#include "tz/wsi/api/monitor.hpp"
#include <vector>

namespace tz::wsi
{
	std::vector<tz::wsi::monitor> get_monitors();
}

#endif // TANGLE_MONITOR_HPP
