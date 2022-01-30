#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include "core/game_info.hpp"
#include "core/window.hpp"

static_assert(TZ_VULKAN || TZ_OGL, "No graphics API detected. You have not configured CMake correctly.");

namespace tz
{
	namespace literals
	{
		inline constexpr unsigned char operator "" _uc(unsigned long long arg) noexcept
		{
		    return static_cast<unsigned char>(arg);
		}
	}

	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * 
	 * This is the only module containing members of the Initial Group. The Initial Group is a group of functions and structures which can be invoked before @ref tz::initialise().
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

	struct InitialiseInfo
	{
		const char* name = "Untitled";
		tz::Version version = {1, 0, 0};
		ApplicationType app_type = ApplicationType::WindowApplication;
		WindowInitArgs window = {};
	};

	/**
	 * @brief Initialise Topaz.
	 * @pre If any other Topaz code not within the "Initial Group" has been invoked before this, then the behaviour of the program is undefined. To know if something is within the Initial Group, check their doucmentation notes.
	 * @post If `tz::terminate()` is not called before the end of program runtime, then the behaviour of the program is undefined.
	 * @details This initialises the render-api backend. Also creates a window unless `ApplicationType::Headless` is specified. Note that for `ApplicationType::HiddenWindowApplication`, a window is created but it is invisible.
	 * 
	 * @param game_info Information about the application
	 * @param app_type Describes the anatomy of the application.
	 */
	void initialise(GameInfo game_info, ApplicationType app_type = ApplicationType::WindowApplication, WindowInitArgs wargs = {});
	void initialise(InitialiseInfo init = {});
	/**
	 * @brief Terminate Topaz.
	 * @pre If `tz::initialise` has not been invoked before this, then the behaviour of the program is undefined.
	 * @post If anything in `tz` is invoked after this, then the behaviour of the program is undefined.
	 * @details This destroys the render-api backend. If a window was created previously by `tz::initialise`, then it will be destroyed here.
	 */
	void terminate();
	/**
	 * @brief Retrieve the application window.
	 * @pre @ref tz::initialise() has been invoked but not with `ApplicationType::Headless`.
	 * @post @ref tz::terminate() has not yet been invoked.
	 * @return Window& Reference to the application window.
	 */
	Window& window();
	/**
	 * @brief Query as to whether Topaz is initialised.
	 * @note This function is within the Initial Group.
	 * @return true If `tz::initialise` has been invoked and `tz::terminate()` has not yet been invoked. Otherwise, returns false.
	 */
	bool is_initialised();
	
	/**
	 * @}
	 */
}
#include "core/tz.inl"

/*! \mainpage Topaz v3.1.0-indev
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
