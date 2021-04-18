#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include "core/game_info.hpp"
#include "core/window.hpp"

namespace tz
{
    enum class ApplicationType
    {
        WindowApplication,
        Headless
    };

    void initialise(GameInfo game_info, ApplicationType app_type = ApplicationType::WindowApplication);
    void terminate();
    Window& window();
    bool is_initialised();
}
#include "core/tz.inl"
#endif // TZ_CORE_TZ_HPP