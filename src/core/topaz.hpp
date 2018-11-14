#ifndef TOPAZ_TOPAZ_HPP
#define TOPAZ_TOPAZ_HPP
#include "audio/audio.hpp"
#include "graphics/graphics.hpp"

#ifdef main
#undef main
#endif

namespace tz
{

    /**
     * Initialise all Topaz modules. tz::graphics is only partially initialised and won't fully initialise until the first Window is instantiated.
     * @param print_progress - Whether to print out initialisation progress or not
     */
    void initialise(bool print_progress = false)
    {
        if(print_progress)
            std::cout << "Initialising Topaz...\n";
        SDL_Init(SDL_INIT_EVERYTHING);
        if(print_progress)
            std::cout << "Initialised SDL2.\n";
        tz::audio::initialise(print_progress);
        if(print_progress)
            std::cout << "Initialised Topaz. Ready to receive OpenGL context...\n";
    }

    /**
     * Terminate all Topaz modules.
     * @param print_progress - Whether to print out termination progress or not
     */
    void terminate(bool print_progress = false)
    {
        if(print_progress)
            std::cout << "Terminating Topaz...\n";
        tz::graphics::terminate(print_progress);
        tz::audio::terminate(print_progress);
        SDL_Quit();
        if(print_progress)
        {
            std::cout << "Terminated SDL2.\n";
            std::cout << "Terminated Topaz.\n";
        }
    }
}

#endif //TOPAZ_TOPAZ_HPP
