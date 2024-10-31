#ifndef TOPAZ_CORE_TIME_HPP
#define TOPAZ_CORE_TIME_HPP
#include <cstdint>

namespace tz
{
	std::uint64_t system_millis();
	std::uint64_t system_nanos();
}

#endif // TOPAZ_CORE_TIME_HPP