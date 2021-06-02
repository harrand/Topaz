/**
 * @page tools Tools
 * This page describes all internal tools used in the Topaz engine.
 * 
 * @section TZSLC TZSLC -- Topaz Shader Language Compiler
 * TZSLC is a tool which preprocesses .tzsl shaders to produce .glsl files appropriate for various render-api GLSL dialects.
 * TZSLC is available under the CMake target `tzslc`. It has no additional documentation.
 * @subsection Usage TZSLC Usage
 * `tzslc <path-to-tzsl-file> [-m{module-name}]...`
 * 
 * @subsubsection tzslc_modules Usage -- Modules
 * You can pass zero or more modules to TZSLC. TZSLC will not preprocess valid .tzsl syntax unless the corresponding module is passed.
 * For example, the module to preprocess image samplers would be called 'sampler', so the tzslc runtime flags would include `-msampler`.
 * 
 * Note that `-mall` is a pseudo-module which causes TZSLC to use all available modules.
 * 
 * <hr>
 * But what's the purpose of TZSLC? Why not just use .glsl shaders and be done with it?
 * @subsection problem Problem: Using an image sampler within a .glsl shader
 * GLSL is somewhat flexible -- Render-apis tend to have their own dialect of GLSL which they use. This means that while often .glsl shaders can be used, for example, both OpenGL and Vulkan - It is not guaranteed to be the case.
 * 
 * TZSLC exists to provide new syntax which preprocesses down to the desired render-api dialect. Consider the following GLSL code to declare a uniform sampler2D:
 * @subsubsection VK Vulkan GLSL
 * `layout(binding = 1) uniform sampler2D tex_sampler;`
 * @subsubsection OGL OpenGL GLSL
 * `layout(location = 1) uniform sampler2D tex_sampler;`
 * @subsection solution Solution: Using a .tzsl shader
 * @subsubsection TZSL Topaz TZSL
 * `tz_sampler2D_uniform(1) tex_sampler;`
 * 
 * Correctly resolves to either of the two outputs above, depending on the build config used to build the given TZSLC process.
 * 
 */