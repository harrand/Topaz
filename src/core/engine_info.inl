#include "core/assert.hpp"
#include <string_view>
#include <cstdint>
#include <iterator>

namespace tz
{
    constexpr EngineInfo info()
    {
        EngineInfo inf{};
        #if TZ_VULKAN
            inf.renderer = EngineInfo::RenderAPI::Vulkan;
        #elif TZ_OGL
            inf.renderer = EngineInfo::RenderAPI::OpenGL;
        #else
            static_assert(false, "tz::info(): No RenderAPI has been set. Can't build.");
        #endif

        #if TZ_DEBUG
            inf.build = EngineInfo::BuildConfig::Debug;
        #else
            inf.build = EngineInfo::BuildConfig::Release;
        #endif
        
        const char* version_string = TZ_VERSION;
        inf.version = tz::parse_version(version_string);
        return inf;
    }
}