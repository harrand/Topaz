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
	 * @defgroup tz_core Core Libraries (tz)
	 * A collection of platform-agnostic core functionality.
	 *
	 * This includes:
	 * - Engine initialisation/termination
	 * - Windowing and hardware requests
	 * - Input processing
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_init Initialisation and Termination
	 * Documentation for functionality related to engine initialisation and termination.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_vecmat Vectors and Matrices
	 * Documentation for vectors and matrices.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_peripherals Hardware and Peripherals
	 * Documentation for retrieving hardware information, such as monitor properties.
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
	 * @ingroup tz_core
	 * @defgroup tz_core_allocators Allocators
	 * Documentation for custom Topaz allocators. All allocators satisfy `std::allocator_traits`.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_profiling Profiling and Instrumentation
	 * Documentation for profiling and instrumentation macros.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_windowing Windowing
	 * Documentation for creating and managing windows.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_utility Utility Helpers
	 * Documentation for utility helper functions and types. This includes 'general' style usage such as handles and callbacks.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_time Times, Dates and sync scheduling
	 * Documentation for functions and helpers which can be used to tell the time, and schedule sync tasks. Has nothing to do with parallellism or threads.
	 */

	/**
	 * @ingroup tz_core_init
	 * Every application is of a single type. When you initialise the engine, you must choose one.
	 */
	enum class ApplicationType
	{
		/// - Initialisation should spawn a window that the user can see and interact with.
		WindowApplication,
		/// - Initialisation should spawn a window, but it is invisible to the user.
		HiddenWindowApplication,
		/// - Initialisation should not spawn a window at all. Headless applications have access to fewer features than others, but do not require a display to initialise. It is recommended to be used for applications used in continuous integration.
		Headless
	};

	/**
	 * @ingroup tz_core_init
	 * Specifies everything needed to initialise the engine.
	 */
	struct InitialiseInfo
	{
		/// - Name of the application. If initialisation spawns a window, the title will contain this string. Defaults to 'Untitled'.
		const char* name = "Untitled";
		/// - Version of the application. If you do not version your application, you can leave this. Defaults to 1.0.0.
		tz::Version version = {1, 0, 0};
		/// - Describes the type of the application. See @ref ApplicationType for more information. Defaults to a window application.
		ApplicationType app_type = ApplicationType::WindowApplication;
		/// - Describes extra information about the window, if initialisation will spawn one. Defaults to a set of implementation-defined flags -- There is no guarantee these will change in subsequent releases so do not rely on the defaults not changing.
		WindowInitArgs window = {};
	};
	/**
	 * @ingroup tz_core_init
	 * Initialise Topaz.
	 * @param init Information about how the application should be initialised.
	 */
	void initialise(InitialiseInfo init = {});
	/**
	 * @ingroup tz_core_init
	 * @brief Terminate Topaz.
	 * @pre If `tz::initialise` has not been invoked before this, then the behaviour of the program is undefined.
	 * @post If anything in `tz` is invoked after this, then the behaviour of the program is undefined.
	 * @details This destroys the render-api backend. If a window was created previously by `tz::initialise`, then it will be destroyed here.
	 */
	void terminate();
	/**
	 * @ingroup tz_core_init
	 * @brief Retrieve the application window.
	 * @pre @ref tz::initialise() has been invoked but not with `ApplicationType::Headless`.
	 * @post @ref tz::terminate() has not yet been invoked.
	 * @return Window& Reference to the application window.
	 */
	Window& window();
	/**
	 * @ingroup tz_core_init
	 * @brief Query as to whether Topaz is initialised.
	 * @note This function is within the Initial Group.
	 * @return true If `tz::initialise` has been invoked and `tz::terminate()` has not yet been invoked. Otherwise, returns false.
	 */
	bool is_initialised();
}
#include "core/tz.inl"

/*! @mainpage Topaz Home
 * # Topaz Documentation
 * ## Introduction
 *
 * Welcome to the home page of the Topaz documentation. Here you can find detailed information about the vast majority of modules, classes and functions within the Topaz engine.
 *
 * If you've come here looking for documentation for a specific topaz technology, you can view documentation for all modules above and peruse at your heart's content. Alternatively, you can search for a specific name on the top-right. Oftentimes you will find usages of most topaz functionality in the existing demos and tests.
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
