# Topaz
## About

Topaz is a multimedia engine written in C++ to be used as a cross-platform abstraction to be used to create applications, games or other programs which demand a modern-C++ mixture of performance and abstraction. Topaz has been in development since it started as an extended project for a sixth-form informal EPQ in 2015.

### Built With

* [GLEW](http://glew.sourceforge.net/) - The OpenGL Extension Wrangler Library, to access OpenGL trivially.
* [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer, cross-platform library used for windowing.
* [SDL2_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/) - SDL2 Plugin used to load audio files (.wav, .ogg files etc...)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/) - SDL2 Plugin used to load TrueType fonts (.ttf files)

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

## Documentation
See the [wiki](https://github.com/Harrand/Topaz/wiki).