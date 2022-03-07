#ifndef TOPAZ_CORE_PROFILING_ZONE_HPP
#define TOPAZ_CORE_PROFILING_ZONE_HPP
#if TZ_PROFILE
#include "Tracy.hpp"
#endif // TZ_PROFILE

/**
 * @ingroup tz_core_profiling
 * @enum TZ_PROFCOL
 * @hideinitializer
 * Represents a colour value for a profiling scope when viewed in the profiling server.
 */
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL TZ_PROFCOL_RED
 * @hideinitializer
 * Bright red zone. Typically used for high-level frontend engine contexts.
 */
#define TZ_PROFCOL_RED 0xFFAA0000
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL TZ_PROFCOL_BLUE
 * @hideinitializer
 * Bright blue zone. Is not used in engine, so you can decide what to use it for.
 */
#define TZ_PROFCOL_BLUE 0xFF0000AA
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL TZ_PROFCOL_GREEN
 * @hideinitializer
 * Bright green zone. Is not used in engine, so you can device what to use it for.
 */
#define TZ_PROFCOL_GREEN 0xFF00AA00
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL TZ_PROFCOL_YELLOW
 * @hideinitializer
 * Golden-yellow zone. Typically used for low-level backend engine contexts.
 */
#define TZ_PROFCOL_YELLOW 0xFFAAAA00

/**
 * @ingroup tz_core_profiling
 * @fn TZ_PROFZONE(name, colour)
 * @param name Name of the zone, this should be a descriptive name with quotes.
 * @param colour Colour of the zone when viewed in the profiling server. See @ref TZ_PROFCOL
 * @hideinitializer
 * Specifies a profiling scope -- Instruments the current scope's timings with the integrated profiling technology.
 * @note Specifying a profiling scope has no effect if `!TZ_PROFILE`
 */
#if TZ_PROFILE
	#define CONCAT_TZPROF(a, b) CONCAT_INNER_TZPROF(a, b)
	#define CONCAT_INNER_TZPROF(a, b) a ## b

	#define UNIQUE_NAME(base) CONCAT_TZPROF(base, __LINE__)
	#define TZ_PROFZONE(name, colour) ZoneNamedNC(UNIQUE_NAME(tracy_profvar), name, colour, true)
#else
	#define TZ_PROFZONE(name, colour)
#endif

/**
 * @ingroup tz_core_profiling
 * @fn TZ_FRAME_BEGIN
 * @hideinitializer
 * Specifies the beginning of a profiling frame -- Instruments the current frame's timings with the integrated profiling technology.
 * @note Specifying a profiling frame has no effect if `!TZ_PROFILE`
 */
/**
 * @ingroup tz_core_profiling
 * @fn TZ_FRAME_END
 * @hideinitializer
 * Specifies the end of a profiling frame -- Instruments the current frame's timings with the integrated profiling technology.
 * @note Specifying a profiling frame has no effect if `!TZ_PROFILE`
 */
#if TZ_PROFILE
	#define TZ_FRAME_BEGIN FrameMarkStart("Frame Loop")
	#define TZ_FRAME_END FrameMarkEnd("Frame Loop")
#else
	#define TZ_FRAME_BEGIN
	#define TZ_FRAME_END
#endif

#endif // TOPAZ_CORE_PROFILING_ZONE_HPP
