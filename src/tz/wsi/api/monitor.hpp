#ifndef TZ_WSI_API_MONITOR_HPP
#define TZ_WSI_API_MONITOR_HPP
#include <string>
#include "hdk/data/vector.hpp"

namespace tz::wsi
{
	struct monitor
	{
		std::string name;
		hdk::vec2ui dimensions;
	};
}

#endif // TZ_WSI_API_MONITOR_HPP
