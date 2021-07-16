#ifndef TOPAZ_CORE_PROFILING_ZONE_HPP
#define TOPAZ_CORE_PROFILING_ZONE_HPP
#if TZ_PROFILE
#include "Tracy.hpp"
#endif // TZ_PROFILE

namespace tz
{
    #define TZ_PROFCOL_RED 0xFFAA0000
    #define TZ_PROFCOL_BLUE 0xFF0000AA
    #define TZ_PROFCOL_GREEN 0xFF00AA00
    #define TZ_PROFCOL_YELLOW 0xFFAAAA00

    #if TZ_PROFILE
        #define CONCAT(a, b) CONCAT_INNER(a, b)
        #define CONCAT_INNER(a, b) a ## b

        #define UNIQUE_NAME(base) CONCAT(base, __LINE__)
        #define TZ_PROFSCOPE(name, colour) ZoneNamedNC(UNIQUE_NAME(tracy_profvar), name, colour, true)
    #else
        #define TZ_PROFSCOPE(name, colour)
    #endif

    #if TZ_PROFILE
        #define TZ_FRAME_BEGIN FrameMarkStart("Frame Loop")
        #define TZ_FRAME_END FrameMarkEnd("Frame Loop")
    #else
        #define TZ_FRAME_BEGIN
        #define TZ_FRAME_END
    #endif
}

#endif // TOPAZ_CORE_PROFILING_ZONE_HPP