#ifndef TOPAZ_TOPAZ_HPP
#define TOPAZ_TOPAZ_HPP
#include "audio/audio.hpp"
#include "graphics/graphics.hpp"
#include "utility/generic.hpp"

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
    /**
     * Initialise all Topaz modules. tz::graphics is only partially initialised and won't fully initialise until the first Window is instantiated.
     * @param print_progress - Whether to print out initialisation progress or not
     */
    void initialise()
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
    void terminate()
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