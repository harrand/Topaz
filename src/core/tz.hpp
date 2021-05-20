#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include "core/game_info.hpp"
#include "core/window.hpp"

namespace tz
{
    enum class ApplicationType
    {
        WindowApplication,
        HiddenWindowApplication,
        Headless
    };

    void initialise(GameInfo game_info, ApplicationType app_type = ApplicationType::WindowApplication);
    void terminate();
    Window& window();
    bool is_initialised();
}
#include "core/tz.inl"

/*! \mainpage Topaz v3.0.0-indev
 * \section intro_sec Introduction
 *
 * Welcome to the Topaz documentation. This was generated automatically via https://github.com/Harrand/Topaz/actions.
 * 
 * If you're here to learn how to use Topaz, it is also recommended to visit the wiki which contains examples and explanations of various engine features. You can find it at: https://github.com/Harrand/Topaz/wiki.
 *
 * \section modules_sec Topaz Modules
 * The Topaz engine is comprised of several large modules. Each have their own responsibility, namespace and location with the source code. For more detailed information about the modules, visit the 'Modules' tab above. A summary of the modules has been provided for you below.
 * <hr>
 * \subsection tzcore Topaz Core
 *
 * The core module of Topaz.
 * 
 * Source location: Topaz/src/core <br>
 * Namespace: `tz`
 * <hr>
 * \subsection tzgl Topaz Graphics Library
 * Low-level 3D graphics functionality. Interacts with a Vulkan or OpenGL frontend, depending on configuration.
 * 
 * Source location: Topaz/src/gl <br>
 * Namespace: `tz::gl`
 * <hr>
 * \image html tz.png width=64px height=64px "Topaz"
 */

#endif // TZ_CORE_TZ_HPP