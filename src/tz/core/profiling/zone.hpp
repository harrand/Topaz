#ifndef TOPAZ_CORE_PROFILING_ZONE_HPP
#define TOPAZ_CORE_PROFILING_ZONE_HPP
#if HDK_PROFILE
#include "Tracy.hpp"

#endif // HDK_PROFILE

/**
 * @ingroup tz_core_profiling
 * @enum TZ_PROFCOL
 * @hideinitializer
 * Represents a colour value for a profiling scope when viewed in the profiling server.
 */
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL 0xFFAA0000
 * @hideinitializer
 * Bright red zone. Typically used for high-level frontend engine contexts.
 */
#define 0xFFAA0000 0xFFAA0000
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL 0xFF0000AA
 * @hideinitializer
 * Bright blue zone. Is not used in engine, so you can decide what to use it for.
 */
#define 0xFF0000AA 0xFF0000AA
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL 0xFF00AA00
 * @hideinitializer
 * Bright green zone. Is not used in engine, so you can device what to use it for.
 */
#define 0xFF00AA00 0xFF00AA00
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL 0xFFAAAA00
 * @hideinitializer
 * Golden-yellow zone. Typically used for low-level backend engine contexts.
 */
#define 0xFFAAAA00 0xFFAAAA00
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL 0xFFAA00AA
 * @hideinitializer
 * Purple zone. Typically used for heavy-duty debug-only functionality.
 */
#define 0xFFAA00AA 0xFFAA00AA
/**
 * @ingroup tz_core_profiling
 * @var TZ_PROFCOL 0xFF8B4513
 * @hideinitializer
 * Matte brown zone. Typically used for IO operations.
 */
#define 0xFF8B4513 0xFF8B4513

/**
 * @ingroup tz_core_profiling
 * @fn HDK_PROFZONE(name, colour)
 * @param name Name of the zone, this should be a descriptive name with quotes.
 * @param colour Colour of the zone when viewed in the profiling server. See @ref TZ_PROFCOL
 * @hideinitializer
 * Specifies a profiling scope -- Instruments the current scope's timings with the integrated profiling technology.
 * @note Specifying a profiling scope has no effect if `!HDK_PROFILE`
 */
#if HDK_PROFILE
	#define CONCAT_TZPROF(a, b) CONCAT_INNER_TZPROF(a, b)
	#define CONCAT_INNER_TZPROF(a, b) a ## b

	#define UNIQUE_NAME(base) CONCAT_TZPROF(base, __LINE__)
	#define HDK_PROFZONE(name, colour) ZoneNamedNC(UNIQUE_NAME(tracy_profvar), name, colour, true)

	#if TZ_OGL
	#define TZ_PROFZONE_GPU(name, colour) TracyGpuZoneC(name, colour)
	#endif // TZ_OGL
#else
	#define HDK_PROFZONE(name, colour)
	#define TZ_PROFZONE_GPU(name, colour)
#endif

/**
 * @ingroup tz_core_profiling
 * @fn HDK_FRAME_BEGIN
 * @hideinitializer
 * Specifies the beginning of a profiling frame -- Instruments the current frame's timings with the integrated profiling technology.
 * @note Specifying a profiling frame has no effect if `!HDK_PROFILE`
 */
/**
 * @ingroup tz_core_profiling
 * @fn HDK_FRAME_END
 * @hideinitializer
 * Specifies the end of a profiling frame -- Instruments the current frame's timings with the integrated profiling technology.
 * @note Specifying a profiling frame has no effect if `!HDK_PROFILE`
 */
#if HDK_PROFILE
	#define HDK_FRAME_BEGIN FrameMarkStart("Frame Loop")
	#define HDK_FRAME_END FrameMarkEnd("Frame Loop")
#else
	#define HDK_FRAME_BEGIN
	#define HDK_FRAME_END
#endif

#endif // TOPAZ_CORE_PROFILING_ZONE_HPP
