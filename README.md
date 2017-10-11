# Topaz

Topaz is a multimedia engine written in C++ to be used as a cross-platform powerful abstraction to hardware components in a modern computer such as the mouse, keyboard, sound card and graphics card. Topaz can be used to create applications, games or other programs which demand a stable mixture of ease-of-use and optimisation.

## Getting Started

There's a DLL in every single build directory. Also an implib in the parent cpl directory. However, both of these are on my .gitignore, so you're actually shit-out-of-luck unless you compile it yourself. If you require assistance to compile this engine, [I have the perfect solution for you](http://lmgtfy.com/?q=learn+c%2B%2B). In all seriousness though, I'm too lazy to make a CMakeLists.txt, although compilation really is almost trivial. I have faith in you.

### Prerequisites

Topaz's default incarnation comes with all required dependencies. However, these dependencies are specifically targeted to the toolchain and software that I, the author, am using during development. Namely, MinGW-W64 x86_64 g++ v7.1.0-posix-seh-rt_v5-rev2. In the likely event that you're not using the exact same software as me, compile the code yourself and have fun compiling SDL2 with Cygwin if you're one of those. If you require assistance with prerequisites, do not hesitate to request my assistance, but know that I am highly likely to respond advising you to run a bath and throw your modem/router into it.

## Deployment

If you wish to use this engine, ask and adhere to the license. If you wish to fork this engine, go fork yourself, use Unreal Engine or something.

## Built With

* [GLEW](http://glew.sourceforge.net/) - The OpenGL Extension Wrangler Library
* [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer
* [SDL2_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/) - SDL2 Plugin used to load audio files (such as .wav files)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/) - SDL2 Plugin used to load TrueType fonts
* [MDL](https://github.com/Harrand/MDL) - Minimalist Data Language

## Contributing

Although I do not currently desire any form of third-party developmental contribution whatsoever, I am open to the contribution of entertainment whilst I feebly continue my pointless existence. Now that I think about it, if you're really reading all of this, chances are I'm not the person with the most pointless existence here. Also I love pictures of puppies, especially golden retrievers. It took much willpower on my behalf to not use a picture of a super cute golden retriever pup as the icon to this engine's testing executable. I'm professional.

## Authors

* **Harry Hollands** - *Sole Developer* - [Harrand](https://github.com/Harrand)

See also the list of [contributors](https://github.com/Harrand/Topaz/contributors) who participated in this project.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE.md](LICENSE.md) file for details.
