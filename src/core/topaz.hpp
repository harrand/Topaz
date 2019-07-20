#ifndef TOPAZ_TOPAZ_HPP
#define TOPAZ_TOPAZ_HPP
#include "audio/audio.hpp"
#include "graphics/graphics.hpp"
#include "utility/generic.hpp"
#include <cassert>

#ifdef main
#undef main
#endif

namespace tz
{
	#ifdef TOPAZ_DEBUG
		constexpr bool is_debug_mode = true;
	#else
		constexpr bool is_debug_mode = false;
	#endif

	#ifdef topaz_assert
		#undef topaz_assert
    #endif
	#define topaz_assert(EXPRESSION, ... ) ((EXPRESSION) ? \
(void)0 : tz::assert_message(std::cerr, \
"Assertion failure: ", #EXPRESSION, "\nIn file: ", __FILE__, \
" on line ", __LINE__, ":\n\t", __VA_ARGS__))
	template<typename... Args>
	void assert_message(std::ostream& out, Args&&... args)
	{
		if constexpr(tz::is_debug_mode)
		{
			std::flush(out);
			(out << ... << args) << std::endl;
			std::abort();
		}
		/*
		#ifndef NDEBUG
			assert((msg_on_failure, expression));
		#else
			if(!expression)
			{
				std::cerr << "tz::assert_that(...) failed: " << msg_on_failure << "\n";
				std::abort();
			}
		#endif
		 */
	}

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