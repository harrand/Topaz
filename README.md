# Topaz
## About

Topaz is a graphics engine written in C++17. The most recent release is [Topaz 1.0](https://github.com/Harrand/Topaz/tree/Topaz1.0). The next release is Topaz 2.0, which has no planned release date.

The Topaz project has been ongoing since it began as a school project in 2015.

### Built With

* [assimp](https://github.com/assimp/assimp) - Open Asset Import Library. Used to load meshes.
* [debugbreak](https://github.com/scottt/debugbreak) - Debugbreak. Used to create breakpoints in code. Compiler-agnostic.
* [GLAD](https://github.com/Dav1dde/glad) - Open Source, multi-language Vulkan/GL/GLES/EGL/GLX/WGL loader-generator based on the official specs. Used to load OpenGL procedures at runtime for use by the engine.
* [GLFW](https://www.glfw.org/) - Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. Used as a cross-platform abstraction for input event processing and windowing.
* [imgui](https://github.com/ocornut/imgui) - Bloat-free graphical user interface library for C++. Used for debug-only user interface for development purposes.
* [stb_image](https://github.com/nothings/stb) - Lightweight image-loading library.
* [CMake](https://cmake.org/) - Open-source, cross-platform family of tools designed to build, test and package software. Used to generate build system files so users can build topaz with the build system of their choice.

### License

Topaz is licensed under the

Apache License 2.0. See LICENSE for more details.

## Installation
### Prerequisites

* OpenGL 4.6 (Earliest tested version)
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