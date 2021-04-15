#ifndef TOPAZ_CORE_GAME_INFO_HPP
#define TOPAZ_CORE_GAME_INFO_HPP
#include "core/tz.hpp"

namespace tz
{
    struct GameInfo
    {
        using Version = tz::EngineInfo::Version;

        constexpr GameInfo(const char* application_name, Version version, EngineInfo engine = tz::info()):
        name(application_name),
        version(version),
        engine(engine){}

        const char* name;
        Version version;
        EngineInfo engine;
    };
}

#endif // TOPAZ_CORE_GAME_INFO_HPP