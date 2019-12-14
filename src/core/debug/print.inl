#include <cstdio>
#include <utility>

namespace tz
{
	template<typename... Args>
	void debug_printf(const char* fmt, Args&&... args)
	{
#if TOPAZ_DEBUG
		std::printf(fmt, std::forward<Args>(args)...);
#endif
	}
}