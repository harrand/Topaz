# Topaz

![Documentation](https://github.com/Harrand/Topaz/actions/workflows/documentation.yml/badge.svg)

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
| 3.1           |Feb 13, 2022|[Topaz3.1](https://github.com/Harrand/Topaz/tree/Topaz3.1)| [Branch: gh-pages-tz3.1](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.1) |
| 3.0           |Sep 17, 2021| [Topaz3.0](https://github.com/Harrand/Topaz/tree/Topaz3.0)|[Branch: gh-pages-tz3](https://github.com/Harrand/Topaz/tree/gh-pages-tz3)|
| 2.0           |May 9, 2021|[Topaz2.0](https://github.com/Harrand/Topaz/tree/Topaz2.0)|[Branch: gh-pages-tz2](https://github.com/Harrand/Topaz/tree/gh-pages-tz2)|
| 1.0 |Sep 25, 2020|[Topaz1.0](https://github.com/Harrand/Topaz/tree/Topaz1.0)|[Branch: gh-pages-tz1](https://github.com/Harrand/Topaz/tree/gh-pages-tz1)|

Note that the [Wiki](https://github.com/Harrand/Topaz/wiki) is only associated with version 3.0 or later.

## Dependencies

All dependencies bar GLAD are submodules. you should checkout the project recursively.

Some dependencies are only required for a specific render-backend and/or build config. See the table below:

|  Render API	| Build Config | Dependency  |
|:-------------:|:------------:|-------------|
| Both          |   Debug      |[debugbreak](https://github.com/scottt/debugbreak)|
| Both          |     Both     |[GLFW](https://www.glfw.org/)|
| Both          |  Profile     |[Tracy](https://github.com/wolfpld/tracy)|
| OpenGL        |     Both     |[GLAD](https://github.com/Dav1dde/glad)|
| Vulkan        |     Both     |[VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)|

This means that you do not need the dependencies if they are not required for your target render-api/build-config. For example, if you are targetting Topaz against OpenGL Release, you do not need VMA nor debugbreak. The intention is to minimise dependencies and only use them when absolutely necessary for a given build config.

## Requirements
The following requirements apply for all possible build configurations:
* A C++20-compliant compiler. GCC, MSVC and Clang are all tested so you can be confident using these. If you're not using any of these compilers, your mileage may vary.
* CMake is required to build. There is not yet a specified minimum version, but unless yours is ancient you should be able to build the engine.
* Unfortunately, neither render-api backends support macOS at present. This means you cannot use Topaz on macOS. However support for Metal is planend in the future, so this may change someday.
#### Vulkan Build
* Windows or Linux.
* Vulkan SDK 1.2 or later must be installed.
* Your graphics card must support Vulkan 1.2. [Check your graphics card here](https://vulkan.gpuinfo.org/).
#### OpenGL Build
* Windows or Linux.
* Your graphics card must support OpenGL 4.6. [Check your graphics card here](https://opengl.gpuinfo.org/).
