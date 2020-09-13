# Topaz 2
## About

Topaz 2 is a rewrite of the [Topaz](https://github.com/Harrand/Topaz) engine. Topaz 2 is an open-source cross-platform graphics/game engine written in C++17.

The Topaz Project has been ongoing since it began as a school project in 2015.

##### Note: Topaz 2 is new and is not yet ready for use. Consider using the legacy Topaz engine. Despite all its flaws, it is usable.

### Built With

* [GLAD](https://github.com/Dav1dde/glad) - Open Source, multi-language Vulkan/GL/GLES/EGL/GLX/WGL loader-generator based on the official specs. Used to load OpenGL procedures at runtime for use by the engine.
* [GLFW](https://www.glfw.org/) - Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. Used as a cross-platform abstraction for input event processing and windowing.
* [CMake](https://cmake.org/) - Open-source, cross-platform family of tools designed to build, test and package software. Used to generate build system files so users can build topaz with the build system of their choice.

### License

TODO: Decide license

Copyright © 2019 [Harrand](https://github.com/Harrand)

## Installation
### Prerequisites

* OpenGL <TODO: insert-version-here>
* Platform which supports all dependencies. See the 'Built With' section above to enumerate these dependencies.
### Compilation

If you have Python, you can use the new provided configure_debug and configure_release scripts. These will automatically configure CMake and choose a build directory for you. You should however provide a CMake generator to their args.

#### Example: Creating MinGW Makefiles in Debug Mode
`> python configure_debug.py MinGW Makefiles`

Will provide a Makefile for all targets which will output build data to `PROJECT_SOURCE_DIR/build/debug`. Note, you should expect to find the resultant Makefile in here too.

The functionality of configure_release.py is identical, except that build data will be outputted to `PROJECT_SOURCE_DIR/build/release`

Invoking CMake manually is a fine approach aswell. You should know of the following defines Topaz expects in the top-level CMakeLists.txt:
* `TOPAZ_DEBUG` should be assigned to 1 if you want to build in Debug, or 0 if you wish to build in Release.

Note: These should ***always*** be defined, it is their value that should be controlled.

## Documentation
See the [wiki](https://github.com/Harrand/Topaz-2/wiki).