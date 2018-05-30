#ifndef TOPAZ_TOPAZ_HPP
#define TOPAZ_TOPAZ_HPP
#include "audio/audio.hpp"
#include "graphics/graphics.hpp"

namespace tz
{
    void initialise()
    {
        tz::util::log::message("Initialising Topaz...");
        SDL_Init(SDL_INIT_EVERYTHING);
        tz::util::log::message("Initialised SDL2.");
        tz::audio::initialise();
        tz::util::log::message("Initialised Topaz. Ready to receive OpenGL context...");
    }

    void terminate()
    {
        tz::util::log::message("Terminating Topaz...");
        tz::graphics::terminate();
        tz::audio::terminate();
        SDL_Quit();
        tz::util::log::message("Terminated SDL2.");
        tz::util::log::message("Terminated Topaz.");
    }
}

#ifdef main
#undef main
#endif

#endif //TOPAZ_TOPAZ_HPP
