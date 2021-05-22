#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include "core/game_info.hpp"
#include "core/window.hpp"

namespace tz
{
    /**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
    /**
     * Describes a given application.
     */
    enum class ApplicationType
    {
        WindowApplication,
        HiddenWindowApplication,
        Headless
    };

    /**
     * @brief Initialise Topaz.
     * @pre If anything in `tz` has been invoked before this, then the behaviour of the program is undefined.
     * @post If `tz::terminate()` is not called before the end of program runtime, then the behaviour of the program is undefined.
     * @details This initialises the render-api backend. Also creates a window unless `ApplicationType::Headless` is specified. Note that for `ApplicationType::HiddenWindowApplication`, a window is created but it is invisible.
     * 
     * @param game_info Information about the application
     * @param app_type Describes the anatomy of the application.
     */
    void initialise(GameInfo game_info, ApplicationType app_type = ApplicationType::WindowApplication);
    /**
     * @brief Terminate Topaz.
     * @pre If `tz::initialise` has not been invoked before this, then the behaviour of the program is undefined.
     * @post If anything in `tz` is invoked after this, then the behaviour of the program is undefined.
     * @details This destroys the render-api backend. If a window was created previously by `tz::initialise`, then it will be destroyed here.
     */
    void terminate();
    /**
     * @brief Retrieve the application window.
     * @pre `tz::initialise` has been invoked
     * @post `tz::terminate()` has not yet been invoked.
     * @return Window& Reference to the application window.
     */
    Window& window();
    /**
     * @brief Query as to whether Topaz is initialised.
     * 
     * @return true If `tz::initialise` has been invoked and `tz::terminate()` has not yet been invoked. Otherwise, returns false.
     */
    bool is_initialised();
    
    /**
	 * @}
	 */
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
 */

#endif // TZ_CORE_TZ_HPP