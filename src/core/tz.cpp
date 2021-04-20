#include "core/tz.hpp"
#include "core/assert.hpp"
#include "core/report.hpp"

#if TZ_VULKAN
#include "gl/vk/impl/tz_vulkan.hpp"
#endif

#include <cstdio>

namespace tz
{
    tz::Window* wnd = nullptr;
    bool initialised = false;
    ApplicationType tz_app_type = ApplicationType::WindowApplication;

    void initialise(GameInfo game_info, ApplicationType app_type)
    {
        glfwInit();
        
        if(app_type == ApplicationType::WindowApplication)
        {
            tz_assert(wnd == nullptr && !initialised, "tz::initialise(): Already initialised");
            wnd = new tz::Window{WindowInitArgs{.width = 800, .height = 600, .title = game_info.name}};
        }
        
        tz_report("%s Application", app_type == ApplicationType::Headless ? "Headless" : "Windowed");

        #if TZ_VULKAN
            if(app_type == ApplicationType::WindowApplication)
            {
                tz::gl::vk::initialise(game_info);
            }
            else
            {
                tz::gl::vk::initialise_headless(game_info);
            }
        #elif TZ_OPENGL

        #endif

        initialised = true;
        tz_app_type = app_type;
    }

    void terminate()
    {
        #if TZ_VULKAN
            tz::gl::vk::terminate();
        #elif TZ_OPENGL

        #endif

        if(tz_app_type == ApplicationType::WindowApplication)
        {
            tz_assert(wnd != nullptr && initialised, "tz::terminate(): Not initialised");
            delete wnd;
        }

        glfwTerminate();
        initialised = false;
    }

    Window& window()
    {
        tz_assert(wnd != nullptr, "tz::window(): Not initialised");
        return *wnd;
    }

    bool is_initialised()
    {
        return initialised;
    }
}