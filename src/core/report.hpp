#include <cstdio>
#include <utility>
#include "core/profiling/zone.hpp"

namespace tz
{
	template<typename... Args>
	void report_internal([[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
		TZ_PROFZONE("tz_report IO", TZ_PROFCOL_BROWN);
		std::fflush(stdout);
		std::fprintf(stdout, fmt, std::forward<Args>(args)...);
	}
}

#ifdef tz_report
#undef tz_report
#endif
/**
 * @ingroup tz_core
 * @fn tz_report(fmt, ...)
 * @hideinitializer
 * Send formatted output to stdout, but also print out the source of the report in the codebase.
 */
#define tz_report(fmt, ...) {tz::report_internal("\"" fmt "\" -- %s:%d\n", ##__VA_ARGS__, __FILE__, __LINE__);} (void)0;

#ifdef tz_warning_report
#undef tz_warning_report
#endif
/**
 * @ingroup tz_core
 * @fn tz_warning_report(fmt, ...)
 * @hideinitializer
 * Functions like @ref tz_report except that the message indicates a warning -- something is wrong but execution should not be interrupted.
 */
#define tz_warning_report(fmt, ...) tz_report("Warning: " fmt, ##__VA_ARGS__)

#ifdef tz_debug_report
#undef tz_debug_report
#endif
/**
 * @ingroup tz_core
 * @fn tz_debug_report(fmt, ...)
 * @hideinitializer
 * Identical to @ref tz_report except that it does nothing on non-debug builds.
 */
#if TZ_DEBUG
	#define tz_debug_report(fmt, ...) tz_report(fmt, ##__VA_ARGS__)
#else
	#define tz_debug_report(fmt, ...) (void)0
#endif
