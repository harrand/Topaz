#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include "tz/core/data/enum_field.hpp"
#include "tz/core/game_info.hpp"
#include "tz/wsi/window.hpp"

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
	 * @defgroup tz_cpp C++ API Reference
	 */

	/**
	 * @defgroup tzsl TZSL API Reference
	 * Topaz Shader Language (@ref shaders)
	 */

	/**
	 * @ingroup tz_cpp
	 * @defgroup tz_core Core Functionality
	 * A grab-bag of common helpers used frequently in other engine modules.
	 *
	 * Notable features:
	 * - Initialisation via @ref tz::initialise() and @ref tz::terminate()
	 * - Callback functions via @ref tz::callback
	 * - Custom file includes (via @ref ImportedTextData)
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_data Data
	 * Documentation for some niché data structures. These do not attempt to replace standard library containers - you should continue using those for the most part, although often one of these containers may be preferable.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_algorithms Algorithms
	 * Documentation for custom Topaz algorithms. These do not attempt to replace standard library algorithms, but in niché circumstances these may come in useful.
	 */

	/**
	 * @ingroup tz_core
	 * Represents custom functionality to be enabled/disabled during initialisation.
	 */
	enum class application_flag
	{
		/// - Disables all graphics and rendering.
		no_graphics,
		/// - Disable all debug-ui functionality.
		no_dbgui,
		/// - The created window is invisible.
		window_hidden,
		/// - The created window cannot be resized.
		window_noresize,
		/// - The created window is transparent, meaning a translucent clear colour is respected.
		window_transparent,
	};

	/// A set of @ref tz::application_flag
	using application_flags = tz::enum_field<application_flag>;

	/**
	 * @ingroup tz_core
	 * Specifies everything needed to initialise the engine.
	 */
	struct initialise_info
	{
		/// - Name of the application. If initialisation spawns a window, the title will contain this string. Defaults to 'Untitled'.
		const char* name = "Untitled";
		/// - Version of the application. If you do not version your application, you can leave this. Defaults to 1.0.0.
		tz::version version = {1, 0, 0, tz::version_type::normal};
		/// - Specifies dimensions of the window, in pixels.
		tz::vec2ui dimensions = {800u, 600u};
		/// - Describes some optional behaviours for the application. See @ref application_flag for details.
		application_flags flags = {};
	};
	/**
	 * @ingroup tz_core
	 * Initialise Topaz, creating a main application window. The main application window is available via @ref tz::window() from now until @ref tz::terminate() is invoked.
	 * @param init Information about how the application should be initialised.
	 */
	void initialise(initialise_info init = {});
	/**
	 * @ingroup tz_core
	 * @brief Terminate Topaz.
	 * @pre If @ref tz::initialise() has not been invoked before this, then the behaviour is undefined.
	 * @details Just how @ref tz::initialise() automatically spawns the main window, `terminate` also automatically destroys the window.
	 */
	void terminate();
	/**
	 * @ingroup tz_core
	 * @brief Begin a new frame.
	 * @pre If @ref tz::initialise() has not been invoked before this, then the behaviour is undefined.
	 */
	void begin_frame();
	/**
	 * @ingroup tz_core
	 * @brief End the current frame.
	 * @pre @ref tz::begin_frame() must have been invoked prior to this, otherwise the behaviour is undefined.
	 */
	void end_frame();
	/**
	 * @ingroup tz_core
	 * @brief Retrieve the application window.
	 * @post @ref tz::initialise() has been invoked prior, but @ref tz::terminate() has not yet been invoked.
	 * @return Reference to the main application window.
	 */
	tz::wsi::window& window();
	/**
	 * @ingroup tz_core
	 * @brief Query as to whether Topaz is initialised.
	 * @note This function is within the Initial Group.
	 * @return true If `tz::initialise` has been invoked and `tz::terminate()` has not yet been invoked. Otherwise, returns false.
	 */
	bool is_initialised();
}
#include "tz/tz.inl"

/*! @mainpage Home
 * ## Introduction
 *
 * Welcome to the home page of the Topaz documentation. Here you can find detailed information about the vast majority of modules, classes and functions within the Topaz engine.
 *
 * If you've come here looking for documentation for a specific topaz technology, you can view documentation for all modules at your heart's content. Oftentimes you will find usages of most topaz functionality in the existing demos and tests.
 *
 * If you're looking for an introduction to the engine, you're best suited to visiting [the wiki](https://github.com/Harrand/Topaz/wiki).
 *
 * ## Links
 * These might come in useful, or you might find some of these interesting.
 * - [Topaz Engine Repository](https://github.com/Harrand/Topaz)
 * - [Topaz Engine Wiki](https://github.com/Harrand/Topaz/wiki)
 * - [Development Playlist (YouTube)](https://www.youtube.com/playlist?list=PL6PSLdrGGe8I67_i6mNk8IUmOsP85Vhll)
 *
 * ## Worked Example
 * The code snippet you're about to see is the source code of the simplest demo, which simply draws a triangle to the screen. This is known as `tz_triangle_demo`.
 * @include gl/tz_triangle_demo.cpp
 */

#endif // TZ_CORE_TZ_HPP
