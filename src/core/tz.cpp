#include "core/tz.hpp"

namespace tz
{
    std::string EngineInfo::to_string() const
    {
        const char* render_api;
        switch(this->renderer)
        {
            case RenderAPI::OpenGL:
                render_api = "OpenGL";
            break;
            case RenderAPI::Vulkan:
                render_api = "Vulkan";
            break;
        }
        const char* build_config;
        switch(this->build)
        {
            case BuildConfig::Debug:
                build_config = "Debug";
            break;
            case BuildConfig::Release:
                build_config = "Release";
            break;
        }
        const char* version_string = TZ_VERSION;

        std::string result = "Topaz v";
        result += version_string;
        result += " ";
        result += render_api;
        result += " ";
        result += build_config;
        return result;
    }
}