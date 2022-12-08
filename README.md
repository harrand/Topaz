# Topaz

[![OGL Debug](https://github.com/Harrand/Topaz/actions/workflows/codebuild_opengl_debug.yml/badge.svg)](https://github.com/Harrand/Topaz/actions/workflows/codebuild_opengl_debug.yml)
[![OGL Release](https://github.com/Harrand/Topaz/actions/workflows/codebuild_opengl_release.yml/badge.svg)](https://github.com/Harrand/Topaz/actions/workflows/codebuild_opengl_release.yml)
[![OGL Tests Debug](https://github.com/Harrand/Topaz/actions/workflows/unittests_opengl_debug.yml/badge.svg)](https://github.com/Harrand/Topaz/actions/workflows/unittests_opengl_debug.yml)
[![OGL Tests Release](https://github.com/Harrand/Topaz/actions/workflows/unittests_opengl_release.yml/badge.svg)](https://github.com/Harrand/Topaz/actions/workflows/unittests_opengl_release.yml)


[![VK Debug](https://github.com/Harrand/Topaz/actions/workflows/codebuild_vulkan_debug.yml/badge.svg)](https://github.com/Harrand/Topaz/actions/workflows/codebuild_vulkan_debug.yml)
[![VK Release](https://github.com/Harrand/Topaz/actions/workflows/codebuild_vulkan_release.yml/badge.svg)](https://github.com/Harrand/Topaz/actions/workflows/codebuild_vulkan_release.yml)

![Documentation](https://github.com/Harrand/Topaz/actions/workflows/documentation.yml/badge.svg)

## About

 Topaz is a 3D graphics engine.
 * C++20
 * Very high-level API, very low-level features.
	* API is specifically designed to allow you to concise define your rendering targets, and which GPU resources you use.
	* This means that you are expected to implement high-level features such as shadow-mapping, SSAO etc... yourself. However, you have absolute control over your GPU buffers and textures.
 * Configurable render backend, currently supporting Vulkan and OpenGL
	* Not coming anytime soon, but planned backends include DX12 and Metal
 
 [Topaz 3.6](https://github.com/Harrand/Topaz/tree/Topaz3.6) is the most recent release of the Topaz Engine. The next version will be version 3.7.

The Topaz project has been ongoing since it began as a school project in 2015.

## Version History
Each major version is a total rewrite and thus completely different from the previous. Each minor version typically consists of API-breaking feature changes. Patch versions are not formally shipped unless they contain emergency bugfixes for a flawed release.
| Topaz Version | Release Date | Branch      | Documentation        |
|:-------------:|:-------------|-------------|----------------------|
| 3.7.0			|Feb 10, 2023 |	master	 | [harrand.github.io](https://harrand.github.io/Topaz/)
| 3.6.0			|Dec 8, 2022 |[Topaz3.6](https://github.com/Harrand/Topaz/tree/Topaz3.6)	 | [Branch: gh-pages-tz3.6](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.6)
| 3.5.0			|Sep 22, 2022|[Topaz3.5](https://github.com/Harrand/Topaz/tree/Topaz3.5)	 | [Branch: gh-pages-tz3.5](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.5)
| 3.4.0			|Aug 3, 2022 |[Topaz3.4](https://github.com/Harrand/Topaz/tree/Topaz3.4)| [Branch: gh-pages-tz3.4](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.4)
| 3.3.0           |Jun 12, 2022|[Topaz3.3](https://github.com/Harrand/Topaz/tree/Topaz3.3)| [Branch: gh-pages-tz3.3](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.3) |
| 3.2.0           |Apr 23, 2022|[Topaz3.2](https://github.com/Harrand/Topaz/tree/Topaz3.2)| [Branch: gh-pages-tz3.2](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.2) |
| 3.1.0           |Feb 13, 2022|[Topaz3.1](https://github.com/Harrand/Topaz/tree/Topaz3.1)| [Branch: gh-pages-tz3.1](https://github.com/Harrand/Topaz/tree/gh-pages-tz3.1) |
| 3.0.0           |Sep 17, 2021| [Topaz3.0](https://github.com/Harrand/Topaz/tree/Topaz3.0)|[Branch: gh-pages-tz3](https://github.com/Harrand/Topaz/tree/gh-pages-tz3)|
| 2.0.0           |May 9, 2021|[Topaz2.0](https://github.com/Harrand/Topaz/tree/Topaz2.0)|[Branch: gh-pages-tz2](https://github.com/Harrand/Topaz/tree/gh-pages-tz2)|
| 1.0.0            |Sep 25, 2020|[Topaz1.0](https://github.com/Harrand/Topaz/tree/Topaz1.0)|[Branch: gh-pages-tz1](https://github.com/Harrand/Topaz/tree/gh-pages-tz1)|

## Build Instructions
1. Checkout the repository resursively.
2. Configure CMake. It is recommended you use a preset.
	
	Example:  `cmake --preset vulkan_debug`
3. Build the `topaz` target to build the engine. Run the `tztest` target to build and run all unit-tests.

## Dependencies
The table below shows all the engine's dependencies.

| Dependency                                      |  Render API	 | Build Config  | Dependency Type |
|-------------------------------------------------|:------------:|:-------------:|-----------------|
[GLFW](https://www.glfw.org/)                      | Both   | All       | Git Submodule
|[GLAD](https://github.com/Dav1dde/glad)           | OpenGL | All       | Included in repository
|[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)  | Vulkan | All       | Requires pre-installation
|[VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)| Vulkan | All       | Git Submodule
|[HDK](https://github.com/Harrand/HDK)| Both   | All      | Git Submodule
[imgui](https://github.com/ocornut/imgui)          | Both   | All    | Git Submodule

## Requirements
The following requirements apply for all possible build configurations:
* A C++20-compliant compiler. GCC, MSVC and Clang are all tested so you can be confident using these. If you're not using any of these compilers, your mileage may vary.
* CMake 3.21 or later is required to build.
#### Vulkan Build
* Windows or Linux.
	* macOS may be theoretically possible now that MoltenVK supports VK1.2, but no investigation has been done to decipher how much work is needed. Definitely, no work is planned for proper Metal support.
* Vulkan SDK 1.2 or later must be installed.

* Your graphics card must support Vulkan 1.2. [Check your graphics card here](https://vulkan.gpuinfo.org/).
	* It must also support the following vulkan extension(s):
		- VK_EXT_descriptor_indexing
#### OpenGL Build
* Windows or Linux.
* Your graphics card must support OpenGL 4.5. [Check your graphics card here](https://opengl.gpuinfo.org/).
	* It must also support the following opengl extension(s):
		- ARB_bindless_texture
			* Note: If this extension is not supported, the engine can still build & run, but ImageResources cannot be used. The unit-tests are guaranteed not to use ImageResources, so those are safe to run if you lack this extension.
