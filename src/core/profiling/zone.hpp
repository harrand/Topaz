#ifndef TOPAZ_CORE_PROFILING_ZONE_HPP
#define TOPAZ_CORE_PROFILING_ZONE_HPP
#include "Tracy.hpp"

namespace tz
{
    #if TZ_PROFILE
        #define TZ_PROFZONE ZoneScoped
        #define TZ_NAMED_PROFZONE(name) ZoneScopedN(name)
    #else
        #define TZ_PROFZONE
        #define TZ_NAMED_PROFZONE
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