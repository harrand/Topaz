#ifndef TOPAZ_HPP
#define TOPAZ_HPP

/**
 * @defgroup tz Topaz API Reference
**/
namespace tz
{
	/**
	 * @ingroup tz
	 * @brief Contains information about the application being initialised.
	 **/
	struct appinfo
	{
		/// Name of the application.
		const char* name = "A Topaz Application";
		// Major version of the application.
		unsigned int major = 1u;
		// Minor version of the application.
		unsigned int minor = 0u;
	};
	/**
	 * @ingroup tz
	 * @brief Initialise the engine.
	 *
	 * You should call this at the start of your program's runtime, before the first Topaz API call you make. You should expect this function to take a significantly long time.
	**/
	void initialise(appinfo info = {});
	/**
	 * @ingroup tz
	 * @brief Terminate the engine, cleaning up all resources.
	 *
	 * You should call this at the end of your program's runtime, after the last Topaz API call you make. You should expect this function to take a significantly long time.
	**/
	void terminate();

	namespace gpu
	{
		void initialise(appinfo info);
		void terminate();
	}
}

/**
 * @ingroup tz
 * @defgroup tz_core Core Libraries
 * @brief Lowest-level libraries, structs and functionality. Everything is expected to depend on this.
**/

/**
 * @ingroup tz
 * @defgroup tz_gpu GPU Library
 * @brief Abstract rendering API for low-level, high-performance 3D desktop graphics. 
 *
 * This library provides a API to interact with a single desktop GPU, agnostic of whichever graphics API runs under-the-hood (e.g Vulkan/OpenGL/D3D12). When you use this library, the implementation will use one of these graphics APIs (which one is used depends on your platform and is configurable at compile-time) automatically with minimal overhead.
 *
 * If you intend to do 3D graphics in Topaz, you should first consult the @ref tz_ren incase a high-level implementation of what you want to achieve already exists. It is unlikely that the @ref tz_ren contains *all* the rendering components you need for a modern game, especially if your game has some more bespoke graphical features or its own unique style. In which case, you should use this library to build your feature from the ground-up.
**/

/**
 * @ingroup tz
 * @defgroup tz_ren Rendering Library
 * @brief High-level declarative rendering API built upon the @ref tz_gpu.
 *
 * This library contains some pre-created high-level rendering systems used for common tasks, such as rendering 3D models, or user interface. These components are intended to be used in most Topaz games, with the goal of not needing to reinvent the wheel in every single game using the @ref tz_ren.
**/

#include "detail/debug.hpp"

#endif // TOPAZ_HPP