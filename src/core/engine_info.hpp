#ifndef TOPAZ_CORE_ENGINE_INFO_HPP
#define TOPAZ_CORE_ENGINE_INFO_HPP
#include <string>

namespace tz
{
    /**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
    /**
     * Represents information about the current engine used.
     * @note This struct is within the Initial Group.
     */
    struct EngineInfo
    {
        enum class RenderAPI
        {
            Vulkan,
            OpenGL
        };

        enum class BuildConfig
        {
            Debug,
            Release
        };

        struct Version
        {
            unsigned int major;
            unsigned int minor;
            unsigned int patch;
        };

        std::string to_string() const;

        RenderAPI renderer;
        BuildConfig build;
        Version version;
    };

    namespace engine_info
    {
        constexpr EngineInfo::Version parse_version(const char* version_string);
        std::string stringify_version(EngineInfo::Version version);
    }

    constexpr EngineInfo info();

    /**
     * @}
     */
}
#include "core/engine_info.inl"
#endif // TOPAZ_CORE_ENGINE_INFO_HPP