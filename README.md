# Topaz

Topaz is a multimedia engine written in C++ to be used as a cross-platform powerful abstraction to hardware components in a modern computer such as the mouse, keyboard, sound card and graphics card. Topaz can be used to create applications, games or other programs which demand a stable mixture of ease-of-use and optimisation.

## PFAQ (Probably Frequency Asked Questions (I haven't been asked any yet and wanted to look clever))
Q: Have you just asked yourself all these questions and answered them yourself like a madman?
A: Yes.
Q: Why the hell are you using `Foo& getFooR()` instead of `void setFoo(Foo f)`? You're breaking encapsulation!
A: My personal style conflicts somewhat with the standard idea of encapsulation. I prefer to expose implementation details of a class if I want it to be editable in anyway, as it avoids verbosity via simply accessing the data member indirectly.
Q: If so, why don't you just make data-members public?
A: With `Foo& getFooR()`, I can make explicitly clear (due to the R) that I intend to edit the value in question whilst having this in a function, allowing me to implement debugging details (such as printing when the variable is accessed) should I choose so. Although I do understand that this style conflicts greatly with standard OO practises and may well ire other programmers, in my own case it's greatly increased productivity all the while I've trusted myself not to be dumb enough to do something silly to the value.
Q: This style is stupid! It's so insecure! How will you be able to verify that a user is using a valid parameter?
A: With this style I indeed cannot easily verify the validity of a parameter. However, this does not disturb me as it may to you: If a parameter is invalid in your setter, you would either do nothing or result to a default. In my view, this seems more like sweeping an error under the rug as opposed to receiving a quick gunshot in the leg for making a mistake.
Q: This is a truly horrific technical style, you should desist.
A: This is a FAQ and that's not a question. That is a truly horrific technical style of questioning, you should desist.
Q: If these questions were all asked by yourself, why are you dissing your own programming style so harshly?
A: So when I commence my bi-weekly debate with myself whether to keep my current style or just use standard getters and setters, I can look back to this PFAQ and consolidate my own stubbornness, hopefully shortly followed by a short chuckle upon seeing how very meta this PFAQ is (and now even more meta because I highlighted that (and even more so now, recursion!)).

## Getting Started

There's a DLL in every single build directory. Also an implib in the parent cpl directory. However, both of these are on my .gitignore, so you're actually shit-out-of-luck unless you compile it yourself. If you require assistance to compile this engine, [I have the perfect solution for you](http://lmgtfy.com/?q=learn+c%2B%2B)

### Prerequisites

Topaz's default incarnation comes with all required dependencies. However, these dependencies are specifically targeted to the toolchain and software that I, the author, am using during development. Namely, MinGW-W64 x86_64 g++ v7.1.0-posix-seh-rt_v5-rev2. In the likely event that you're not using the exact same software as me, compile the code yourself and have fun compiling SDL2 with Cygwin if you're one of those. If you require assistance with prerequisites, do not hesitate to request my assistance, but know that I am highly likely to respond advising you to run a bath and throw your modem/router into it.

## Deployment

If you wish to use this engine, ask and adhere to the license. If you wish to fork this engine, go fork yourself, use Unreal Engine or something.

## Built With

* [GLEW](http://glew.sourceforge.net/) - The OpenGL Extension Wrangler Library
* [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer
* [SDL2_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/) - SDL2 Plugin used to load audio files (such as .wav files)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/) - SDL2 Plugin used to load TrueType fonts

## Contributing

Although I do not currently desire any form of third-party developmental contribution whatsoever, I am open to the contribution of entertainment whilst I feebly continue my pointless existence. Now that I think about it, if you're really reading all of this, chances are I'm not the person with the most pointless existence here. Also I love pictures of puppies, especially golden retrievers. It took much willpower on my behalf to not use a picture of a super cute golden retriever pup as the icon to this engine's testing executable. I'm professional.

## Versioning

I use [Git](http://github.com/) for versioning. Bet you didn't see that one coming. For the versions available, see the [tags on this repository](https://github.com/Harrand/Topaz/tags). 

## Authors

* **Harry Hollands** - *Sole Developer* - [Harrand](https://github.com/Harrand)

See also the list of [contributors](https://github.com/Harrand/Topaz/contributors) who participated in this project.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* Blizzard Entertainment for replacing my social life with an outright love for development
* University of Nottingham for giving me so much time off in the summer that I've been able to spend alot of it being unprofessional and unproductive, which is, of course, what you're reading right now when I should be developing this engine more.
