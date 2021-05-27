# Topaz
![tz_triangle_demo linux-opengl-debug](https://github.com/Harrand/Topaz/actions/workflows/build_tz_triangle_demo.yml/badge.svg)
![non_graphical_tests linux-opengl-debug](https://github.com/Harrand/Topaz/actions/workflows/non_graphical_tests.yml/badge.svg)

## About

 Topaz is a graphics engine written in C++20. Topaz 3.0 is currently in development. [Topaz 2.0](https://github.com/Harrand/Topaz/tree/Topaz2.0) is the most recent release of the Topaz Engine.

The Topaz project has been ongoing since it began as a school project in 2015.

## Documentation and Wiki
Documentation for Topaz 3.0 can be found [here](https://harrand.github.io/Topaz/).

### Built With

* [debugbreak](https://github.com/scottt/debugbreak) - Debugbreak. Used to create breakpoints in code. Compiler-agnostic.
* [GLAD](https://github.com/Dav1dde/glad) - Open Source, multi-language Vulkan/GL/GLES/EGL/GLX/WGL loader-generator based on the official specs. Used to load OpenGL procedures at runtime for use by the engine.
* [GLFW](https://www.glfw.org/) - Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. Used as a cross-platform abstraction for input event processing and windowing.
* [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Memory allocation library for Vulkan.

## Installation Requirements
Requirements to build Topaz are listed below. Some requirements are specific to a certain build configuration.
* Supports GCC, Clang and MSVC.
* Supports Windows, macOS, x11 and Wayland.
* CMake is required to build. There is not yet a specified minimum version, but unless yours is ancient you should be able to build the engine.
#### Vulkan Build
* Vulkan 1.1.175 or later
#### OpenGL Build
* OpenGL 4.6 or later

## Version History
Please note that documentation is not published for old versions of Topaz. You must retrieve it yourself from the relevant branch.
### Topaz 2.0
* [Branch available here](https://github.com/Harrand/Topaz/tree/Topaz2.0)
#### Retrieving Documentation
Documentation for Topaz 2.0 is automatically published at the branch [gh-pages-tz2](https://github.com/Harrand/Topaz/tree/gh-pages-tz2). To retrieve documentation, run the following command:
`git clone https://github.com/Harrand/Topaz.git --branch gh-pages-tz2 --single-branch`
### Topaz 1.0
* [Branch available here](https://github.com/Harrand/Topaz/tree/Topaz1.0)
#### Retrieving Documentation
Documentation for Topaz 1.0 is automatically published at the branch [gh-pages-tz1](https://github.com/Harrand/Topaz/tree/gh-pages-tz1). To retrieve documentation, run the following command:
`git clone https://github.com/Harrand/Topaz.git --branch gh-pages-tz1 --single-branch`
