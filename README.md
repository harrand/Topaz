# tz_vk

## About

tz_vk is a special repository. [Topaz 2.0](https://github.com/Harrand/Topaz/tree/Topaz2.0) is the most recent version of the Topaz Engine. Topaz 3.0 is currently in development, represented by this repository. This is not a branch of Topaz but a separate repository due to the fact that this is essentially a complete rewrite -- This whole rewrite could be trashed at a moments notice without any adverse affects on Topaz's git history.

The Topaz project has been ongoing since it began as a school project in 2015.

## Documentation and Wiki
tz_vk has no documentation nor wiki.

### Built With

* [debugbreak](https://github.com/scottt/debugbreak) - Debugbreak. Used to create breakpoints in code. Compiler-agnostic.
* [GLAD](https://github.com/Dav1dde/glad) - Open Source, multi-language Vulkan/GL/GLES/EGL/GLX/WGL loader-generator based on the official specs. Used to load OpenGL procedures at runtime for use by the engine.
* [GLFW](https://www.glfw.org/) - Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. Used as a cross-platform abstraction for input event processing and windowing.
* [CMake](https://cmake.org/) - Open-source, cross-platform family of tools designed to build, test and package software. Used to generate build system files so users can build topaz with the build system of their choice.


## Installation
### Prerequisites
* Platform which supports all dependencies. See the 'Built With' section above to enumerate these dependencies.
#### Vulkan Build
* Vulkan 1.1.175 or later
#### OpenGL Build
* OpenGL 4.6 (as of Topaz2.0 - version requirement for tz_vk not yet clear)