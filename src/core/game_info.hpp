#ifndef TOPAZ_CORE_GAME_INFO_HPP
#define TOPAZ_CORE_GAME_INFO_HPP
#include "core/engine_info.hpp"

namespace tz
{    
    /**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
    /**
     * Contains basic information about the target game/application.
     * @note This struct is within the Initial Group.
     */
    struct GameInfo
    {
        using Version = tz::EngineInfo::Version;

        constexpr GameInfo(const char* application_name, Version version, EngineInfo engine = tz::info()):
        name(application_name),
        version(version),
        engine(engine){}

        /**
         * @brief Retrieve a string describing the GameInfo.
         * - See @ref EngineInfo::to_string() for `ENGINEINFO`
         * @return 'APPNAME vX.Y.Z (`ENGINEINFO`)'
         */
        inline std::string to_string() const
        {
            std::string res = this->name;
            res += " v";
            res += engine_info::stringify_version(this->version);
            res += " (";
            res += this->engine.to_string();
            res += ")";
            return res;
        }

        const char* name;
        Version version;
        EngineInfo engine;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_CORE_GAME_INFO_HPP