#ifndef TOPAZ_CORE_PROFILING_ZONE_HPP
#define TOPAZ_CORE_PROFILING_ZONE_HPP
#include "Tracy.hpp"

namespace tz
{
    #define TZ_PROFCOL_RED 0xFFAA0000
    #define TZ_PROFCOL_BLUE 0xFF0000AA
    #define TZ_PROFCOL_GREEN 0xFF00AA00
    #define TZ_PROFCOL_YELLOW 0xFFAAAA00

    #if TZ_PROFILE
        #define TZ_PROFSCOPE(name, colour) ZoneScopedNC(name, colour)
        #define TZ_PROFZONE TZ_PROFSCOPE(__func__, 0xFFFFFFFF)
    #else
        #define TZ_PROFSCOPE
        #define TZ_PROFZONE
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