#ifndef TZ_WSI_MONITOR_HPP
#define TZ_WSI_MONITOR_HPP
#include "tz/wsi/api/monitor.hpp"
#include <vector>

namespace tz::wsi
{
	/**
	 * @ingroup tz_wsi_monitor
	 * Retrieve a list of all hardware monitors currently connected to the machine.
	 * @note The primary monitor is guaranteed to be the first element of this list.
 	 */
	std::vector<tz::wsi::monitor> get_monitors();
}

#endif // TZ_WSI_MONITOR_HPP
