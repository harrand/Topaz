# Topaz
![tz_triangle_demo gcc-opengl-debug](https://github.com/Harrand/Topaz/actions/workflows/build_tz_triangle_demo.yml/badge.svg)

![non_graphical_tests gcc-opengl-debug](https://github.com/Harrand/Topaz/actions/workflows/non_graphical_tests_opengl_gcc.yml/badge.svg)
![non_graphical_tests clang-opengl-debug](https://github.com/Harrand/Topaz/actions/workflows/non_graphical_tests_opengl_clang.yml/badge.svg)
![non_graphical_tests windows-opengl-debug](https://github.com/Harrand/Topaz/actions/workflows/non_graphical_tests_opengl_msvc.yml/badge.svg)

## About

 Topaz is a 3D graphics engine.
 * C++20
 * Focus on ease-of-use with plenty of abstraction
 * Configurable render backend, currently supporting Vulkan and OpenGL
	* Planned backends include DX12 and Metal
	* Vulkan configuration fully supports headless rendering
 * Strongly adheres to OOP
 
 [Topaz 3.1](https://github.com/Harrand/Topaz/tree/Topaz3.1) is the most recent release of the Topaz Engine. The next version will be version 3.2.

The Topaz project has been ongoing since it began as a school project in 2015.

## Version History

| Topaz Version | Release Date | Branch      | Documentation        |
|:-------------:|:-------------|-------------|----------------------|
| 3.2           |TBA|[master](https://github.com/Harrand/Topaz/tree/master)| [harrand.github.io](https://harrand.github.io/Topaz/) |
| 3.1           |Feb 13, 2022|[Topaz3.1](https://github.com/Harrand/Topaz/tree/Topaz3.1)| [harrand.github.io](https://harrand.github.io/Topaz/) |
| 3.0           |Sep 17, 2021| [Topaz3.0](https://github.com/Harrand/Topaz/tree/Topaz3.0)|[Branch: gh-pages-tz3](https://github.com/Harrand/Topaz/tree/gh-pages-tz3)|
| 2.0           |May 9, 2021|[Topaz2.0](https://github.com/Harrand/Topaz/tree/Topaz2.0)|[Branch: gh-pages-tz2](https://github.com/Harrand/Topaz/tree/gh-pages-tz2)|
| 1.0 |Sep 25, 2020|[Topaz1.0](https://github.com/Harrand/Topaz/tree/Topaz1.0)|[Branch: gh-pages-tz1](https://github.com/Harrand/Topaz/tree/gh-pages-tz1)|

Note that the [Wiki](https://github.com/Harrand/Topaz/wiki) is only associated with version 3.0 or later.

## Dependencies

All dependencies are included in project - There are no submodules. This means you don't need to install any of these yourself.

* [debugbreak](https://github.com/scottt/debugbreak) - Debugbreak. Used to create breakpoints in code. Compiler-agnostic.
* [GLAD](https://github.com/Dav1dde/glad) - Open Source, multi-language Vulkan/GL/GLES/EGL/GLX/WGL loader-generator based on the official specs.
	* This is only used in the OpenGL build.
* [GLFW](https://www.glfw.org/) - Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. Used as a cross-platform abstraction for input event processing and windowing.
* [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Memory allocation library for Vulkan.
	* This is only used in the Vulkan build.

## Requirements
The following requirements apply for all possible build configurations:
* A C++20-compliant compiler. GCC, MSVC and Clang are all tested so you can be confident using these. If you're not using any of these compilers, your mileage may vary.
* CMake is required to build. There is not yet a specified minimum version, but unless yours is ancient you should be able to build the engine.
#### Vulkan Build
* Windows, macOS or Linux
* Vulkan SDK 1.1.175 or later must be installed
* Your graphics card must support Vulkan 1.1.175. [Check your graphics card here](https://vulkan.gpuinfo.org/).
#### OpenGL Build
* Windows or Linux. If you are on macOS, you must use the Vulkan build - Apple dropped support since OpenGL 4.1.
* Your graphics card must support OpenGL 4.6. [Check your graphics card here](https://opengl.gpuinfo.org/).
