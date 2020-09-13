#include <cstdio>
#include <utility>

namespace tz
{
	template<typename... Args>
	void debug_printf([[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
#if TOPAZ_DEBUG
		std::printf(fmt, std::forward<Args>(args)...);
#endif
	}
}