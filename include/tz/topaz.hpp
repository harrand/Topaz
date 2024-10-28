#ifndef TOPAZ_HPP
#define TOPAZ_HPP
#include "textc/imported_text.hpp"

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
		/// Major version of the application.
		unsigned int major = 1u;
		/// Minor version of the application.
		unsigned int minor = 0u;
	};
	/**
	 * @ingroup tz
	 * @brief Initialise the engine.
	 * @param info Some basic information about your application. If you don't provide this, some placeholder values will be provided for you.
	 *
	 * This function *must* be the first Topaz API call you make, ideally at the start of your program's runtime.
	 *
	 * You should expect this function to take a significantly long time.
	**/
	void initialise(appinfo info = {});
	/**
	 * @ingroup tz
	 * @brief Terminate the engine, cleaning up all resources.
	 *
	 * This function should be the last Topaz API call you make before the end of your program's runtime.
	 *
	 * You should expect this function to take a significantly long time.
	**/
	void terminate();

	namespace detail
	{
		void job_system_initialise();
		void job_system_terminate();
		void lua_initialise_local();
		void lua_initialise_all_threads();
	}
	namespace gpu
	{
		void initialise(appinfo info);
		void terminate();
	}
	namespace os
	{
		void initialise();
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
 * @defgroup tz_os OS Libraries
 * @brief Cross-platform API to interact with operating-system level components, such as windowing, hardware input, and machine non-GPU hardware.
**/

/**
 * @ingroup tz
 * @defgroup tz_gpu GPU Library
 * @brief Abstract rendering API for low-level, high-performance 3D desktop graphics. 
 *
 * This library provides a API to interact with a single desktop GPU, agnostic of whichever graphics API runs under-the-hood (e.g Vulkan/OpenGL/D3D12). When you use this library, the implementation will use one of these graphics APIs (which one is used depends on your platform and is configurable at compile-time) automatically with minimal overhead.
 *
 * If you intend to do 3D graphics in Topaz, you should first consult the @ref tz_ren incase a high-level implementation of what you want to achieve already exists. It is unlikely that the @ref tz_ren contains *all* the rendering components you need for a modern game, especially if your game has some more bespoke graphical features or its own unique style. In which case, you should use this library to build your feature from the ground-up.
 *
 *
 * The @ref tz_gpu works as follows:
 * - @ref tz_gpu_pass are comprised of:
 *		- @ref tz_gpu_shader - that is, a single shader program.
 * 		- Zero or more @ref tz_gpu_resource - buffers and images that can be accessed within the shader.
 * - You can build @ref tz_gpu_graph which are comprised of any number of existing passes. You can choose the execution order of these passes, aswell as any dependencies (if Pass X depends on Pass Y, then Pass Y will not begin execution until Pass X has completed all of its GPU work).
 * - Every frame, you can invoke a single graph.
 *
 * An example graph could be comprised of:
 * - 1.) A frustum/occlusion culling pass using a compute shader.
 * - 2.) A heavy-duty 3D animation rendering pass to render all the animated models geometry in the scene into a gbuffer comprised of multiple image resources. Depends on (1)
 * - 3.) A text-rendering pass to render all the game's text into a single image resource.
 * - 4.) A post-processing pass to pixelate the colour attachment from (2) to provide retro PS1 aesthetics. Depends on (2)
 * - 5.) A deferred shading pass to add lighting detail to the scene, combining it with the text rendered earlier. This is outputted into the internal window image. Depends on (3, 4).
 * - 6.) Present the final window image. Depends on (5).
 *
 * The timeline of GPU execution of this graph could look as follows:
 * - 1 and 3 both begin execution at the start of the frame.
 * - 3 completes.
 * - 1 completes. 2 now begins execution.
 * - 2 completes. 4 now begins execution.
 * - 4 completes. 5 now begins execution.
 * - 5 completes. 6 executes and the final image is displayed in the window.
**/

/**
 * @ingroup tz
 * @defgroup tz_ren Rendering Library
 * @brief High-level declarative rendering API built upon the @ref tz_gpu.
 *
 * This library contains some pre-created high-level rendering systems used for common tasks, such as rendering 3D models, or user interface. These components are intended to be used in most Topaz games, with the goal of not needing to reinvent the wheel in every single game using the @ref tz_ren.
**/

/**
 * @ingroup tz
 * @defgroup tz_io IO Library
 * @brief High-level library for wrangling files of various formats.
 *
 * This library contains support for a handful of common file formats, allowing you to import them without having to implement a parser yourself.
**/


#include "detail/debug.hpp"

#endif // TOPAZ_HPP