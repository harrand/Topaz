#include <string>
#include <cstdio>

int main()
{
    std::string render_api;
    std::string build_config;
#if TZ_VULKAN
    render_api += "Vulkan";
#endif
#if TZ_OGL
    render_api += "OpenGL";
#endif
#if TZ_DEBUG
    build_config = "Debug";
#else
    build_config = "Release"
#endif
    std::printf("RenderAPI: %s, Config: %s", render_api.c_str(), build_config.c_str());
    return 0;
}