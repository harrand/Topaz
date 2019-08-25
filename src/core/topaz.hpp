#ifndef TOPAZ_TOPAZ_HPP
#define TOPAZ_TOPAZ_HPP
#include "audio/audio.hpp"
#include "graphics/graphics.hpp"
#include "utility/generic.hpp"
#include "core/assert.hpp"

#ifdef main
#undef main
#endif

namespace tz
{
	enum class GraphicsAPI
	{
		OPENGL,
		VULKAN
	};

	#ifdef TOPAZ_OPENGL
		constexpr GraphicsAPI graphics_api = GraphicsAPI::OPENGL;
	#elif TOPAZ_VULKAN
		constexpr GraphicsAPI graphics_api = GraphicsAPI::VULKAN;
	#else
		static_assert(false, "Neither 'TOPAZ_OPENGL' or 'TOPAZ_VULKAN' defined in compile flags.");
	#endif
	/**
	 * Initialise all Topaz modules. tz::graphics is only partially initialised and won't fully initialise until the first Window is instantiated.
	 * @param print_progress - Whether to print out initialisation progress or not
	 */
	inline void initialise()
	{
		tz::debug::print("tz::initialise(): Initialising Topaz...\n");
		SDL_Init(SDL_INIT_EVERYTHING);
		tz::debug::print("tz::initialise(): Initialised SDL2.\n");
		tz::audio::initialise();
		tz::debug::print("tz::initialise(): Initialised Topaz. Ready to receive OpenGL context...\n");
	}

	/**
	 * Terminate all Topaz modules.
	 * @param print_progress - Whether to print out termination progress or not
	 */
	inline void terminate()
	{
		tz::debug::print("tz::terminate(): Terminating Topaz...\n");
		tz::graphics::terminate();
		tz::audio::terminate();
		SDL_Quit();
		tz::debug::print("tz::terminate(): Terminated SDL2.\n");
		tz::debug::print("tz::terminate(): Terminated Topaz.\n");
	}
}

#endif //TOPAZ_TOPAZ_HPP