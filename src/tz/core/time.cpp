#include "tz/core/time.hpp"
#include <chrono>

namespace tz
{
	std::uint64_t time_millis()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	std::uint64_t time_nanos()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}
}