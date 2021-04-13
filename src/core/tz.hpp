#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include <string>

namespace tz
{
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

    constexpr EngineInfo info();
}
#include "core/tz.inl"
#endif // TZ_CORE_TZ_HPP