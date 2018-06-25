#ifndef TOPAZ_TOPAZ_HPP
#define TOPAZ_TOPAZ_HPP
#include "audio/audio.hpp"
#include "graphics/graphics.hpp"

#ifdef main
#undef main
#endif

namespace tz
{
    void initialise(bool print_progress = false)
    {
        if(print_progress)
            tz::util::log::message("Initialising Topaz...");
        SDL_Init(SDL_INIT_EVERYTHING);
        if(print_progress)
        {
            tz::util::log::message("Initialised SDL2.");
            tz::audio::initialise(print_progress);
            tz::util::log::message("Initialised Topaz. Ready to receive OpenGL context...");
        }
    }

    void terminate(bool print_progress = false)
    {
        if(print_progress)
            tz::util::log::message("Terminating Topaz...");
        tz::graphics::terminate(print_progress);
        tz::audio::terminate(print_progress);
        SDL_Quit();
        if(print_progress)
        {
            tz::util::log::message("Terminated SDL2.");
            tz::util::log::message("Terminated Topaz.");
        }
    }
}

#endif //TOPAZ_TOPAZ_HPP
