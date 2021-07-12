#include "core/engine_info.hpp"

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
            case BuildConfig::Profile:
                build_config = "Profile";
            break;
            case BuildConfig::Release:
                build_config = "Release";
            break;
        }
        std::string result = "Topaz v";
        result += tz::stringify_version(this->version);
        result += " ";
        result += render_api;
        result += " ";
        result += build_config;
        return result;
    }

}