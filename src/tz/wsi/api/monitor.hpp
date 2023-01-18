#ifndef TZ_WSI_API_MONITOR_HPP
#define TZ_WSI_API_MONITOR_HPP
#include <string>
#include "hdk/data/vector.hpp"

namespace tz::wsi
{
	/**
	 * @ingroup tz_wsi_monitor
	 * Represents information about a monitor.
	 */ 
	struct monitor
	{
		/// Implementation-defined name. Not guaranteed to be unique.
		std::string name;
		/// Dimensions of the monitor, in pixels.
		hdk::vec2ui dimensions;
	};
}

#endif // TZ_WSI_API_MONITOR_HPP
