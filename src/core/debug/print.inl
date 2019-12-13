#include <cstdio>
#include <utility>

namespace tz
{
    template<typename... Args>
    void debug_printf(const char* fmt, Args&&... args)
    {
        std::printf(fmt, std::forward<Args>(args)...);
    }
}