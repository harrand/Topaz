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
    /**
     * Initialise all Topaz modules. tz::graphics is only partially initialised and won't fully initialise until the first Window is instantiated.
     * @param print_progress - Whether to print out initialisation progress or not
     */
    void initialise()
    {
        tz::debug::print("Initialising Topaz...\n");
        SDL_Init(SDL_INIT_EVERYTHING);
        tz::debug::print("Initialised SDL2.\n");
        tz::audio::initialise();
        tz::debug::print("Initialised Topaz. Ready to receive OpenGL context...\n");
    }

    /**
     * Terminate all Topaz modules.
     * @param print_progress - Whether to print out termination progress or not
     */
    void terminate()
    {
        tz::debug::print("Terminating Topaz...\n");
        tz::graphics::terminate();
        tz::audio::terminate();
        SDL_Quit();
        tz::debug::print("Terminated SDL2.\n");
        tz::debug::print("Terminated Topaz.\n");
    }
}

#endif //TOPAZ_TOPAZ_HPP