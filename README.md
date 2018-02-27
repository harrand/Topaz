# Topaz
## About

Topaz is a multimedia engine written in C++ to be used as a cross-platform powerful abstraction to hardware components in a modern computer such as the mouse, keyboard, sound card and graphics card. Topaz can be used to create applications, games or other programs which demand a unique mixture of performance and abstraction.

### Built With

* [GLEW](http://glew.sourceforge.net/) - The OpenGL Extension Wrangler Library
* [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer
* [SDL2_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/) - SDL2 Plugin used to load audio files (such as .wav files)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/) - SDL2 Plugin used to load TrueType fonts
* [MDL](https://github.com/Harrand/MDL) - Minimalist Data Language

### License

This project is licensed under the Apache License 2.0 - see the [LICENSE.md](LICENSE.md) file for details.

## Installation
### Prerequisites

* OpenGL v4.3 or later. See your graphics card information to verify that you support this.
* SDL2-supported hardware. See [their wiki on installation](http://wiki.libsdl.org/Installation) to verify this.
### Compilation

If you have CMake:

See `CMakeLists.txt` in the root directory.

If you do not, see the windows batch files (.bat) included in the root folder to aid with compilation. If you are not on Windows nor currently have CMake, then you must [install CMake](https://cmake.org/) to use this library.