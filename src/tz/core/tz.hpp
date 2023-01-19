#ifndef TZ_CORE_TZ_HPP
#define TZ_CORE_TZ_HPP
#include "tz/core/containers/enum_field.hpp"
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
	 * @defgroup tz_core_containers Containers
	 * Documentation for custom Topaz containers, such as BasicList and EnumField.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_algorithms Algorithms
	 * Documentation for custom Topaz algorithms.
	 */


	/**
	 * @defgroup tzsl Topaz Shading Language (TZSL)
	 * Documentation for the TZSL language and standard library.
	 */

	enum class application_flag
	{
		window_hidden,
		window_noresize,
		window_transparent,
	};

	using application_flags = tz::EnumField<application_flag>;

	/**
	 * @ingroup tz_core
	 * Specifies everything needed to initialise the engine.
	 */
	struct initialise_info
	{
		/// - Name of the application. If initialisation spawns a window, the title will contain this string. Defaults to 'Untitled'.
		const char* name = "Untitled";
		/// - Version of the application. If you do not version your application, you can leave this. Defaults to 1.0.0.
		hdk::version version = {1, 0, 0, hdk::version_type::normal};
		/// - Specifies dimensions of the window, in pixels.
		hdk::vec2ui dimensions = {800u, 600u};
		/// - Describes some optional behaviours for the application. See @ref application_flag for details.
		application_flags flags = {};
	};
	/**
	 * @ingroup tz_core
	 * Initialise Topaz.
	 * @param init Information about how the application should be initialised.
	 */
	void initialise(initialise_info init = {});
	/**
	 * @ingroup tz_core
	 * @brief Terminate Topaz.
	 * @pre If `tz::initialise` has not been invoked before this, then the behaviour of the program is undefined.
	 * @post If anything in `tz` is invoked after this, then the behaviour of the program is undefined.
	 * @details This destroys the render-api backend. If a window was created previously by `tz::initialise`, then it will be destroyed here.
	 */
	void terminate();
	void begin_frame();
	void end_frame();
	/**
	 * @ingroup tz_core
	 * @brief Retrieve the application window.
	 * @post @ref tz::terminate() has not yet been invoked.
	 * @return Window& Reference to the application window.
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
#include "tz/core/tz.inl"

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
