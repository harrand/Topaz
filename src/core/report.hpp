#include <cstdio>
#include <utility>

namespace tz
{
    template<typename... Args>
    void report_internal(const char* fmt, Args&&... args)
    {
        std::fflush(stdout);
        std::fprintf(stdout, fmt, std::forward<Args>(args)...);
    }
}

#ifdef tz_report
#undef tz_report
#endif
#define tz_report(fmt, ...) {tz::report_internal("\"" fmt "\" -- %s:%d\n", ##__VA_ARGS__, __FILE__, __LINE__);} (void)0;

#ifdef tz_debug_report
#undef tz_debug_report
#endif
#if TZ_DEBUG
    #define tz_debug_report(fmt, ...) tz_report(fmt, ##__VA_ARGS__)
#else
    #define tz_debug_report(fmt, ...) (void)0
#endif