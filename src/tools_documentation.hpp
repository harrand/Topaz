/**
 * @page tools Tools
 * This page describes all internal tools used in the Topaz engine.
 * 
 * @section TZSLC TZSLC -- Topaz Shader Language Compiler
 * TZSLC is a tool which transpiles .tzsl shaders to produce output files appropriate for various render-api GLSL dialects.
 * - In a Vulkan environment, TZSLC produces SPIR-V binaries.
 * - In an OpenGL environment, TZSLC produces GLSL sources.
 * 
 * TZSLC is available under the CMake target `tzslc`. It has no additional documentation.
 * @subsection TZSL
 * TZSL stands for TopaZ Shader Language. Files containing TZSL code typically end in .tzsl. It is virtually identical to GLSL aside from a few quality-of-life changes:
 * - Any TZSL code containing a `#version` directive is ill-formed -- This is a GLSL directive that specifies the version of GLSL used. When TZSLC produces the GLSL output, it chooses the GLSL version used.
 * - GLSL is not consistent across render-apis. Vulkan and OpenGL each have their own dialect, meaning that often the same .glsl file cannot be used for both render-apis. TZSL exists to mitigate this issue.
 * @subsection Usage TZSLC Usage
 * `tzslc <path-to-tzsl-file> [-m{module-name}]...`
 * 
 * @subsubsection tzslc_modules Usage -- Modules
 * You can pass zero or more modules to TZSLC. TZSLC will not preprocess valid .tzsl syntax unless the corresponding module is passed.
 * For example, the module to preprocess image samplers would be called 'sampler', so the tzslc runtime flags would include `-msampler`.
 * 
 * Note that `-mall` is a pseudo-module which causes TZSLC to use all available modules.
 * 
 * But what's the purpose of TZSLC? Why not just use .glsl shaders and be done with it?
 * <hr>
 * @subsection problem1 Problem: Using an image sampler within a .glsl shader
 * GLSL is somewhat flexible -- Render-apis tend to have their own dialect of GLSL which they use. This means that while often .glsl shaders can be used, for example, both OpenGL and Vulkan - It is not guaranteed to be the case.
 * 
 * TZSLC exists to provide new syntax which preprocesses down to the desired render-api dialect. Consider the following GLSL code to declare a uniform sampler2D:
 * @subsubsection VK Vulkan GLSL
 * `layout(binding = 1) uniform sampler2D tex_sampler;`
 * @subsubsection OGL OpenGL GLSL
 * `layout(location = 1) uniform sampler2D tex_sampler;`
 * @subsection solution1 Solution: Using a .tzsl shader
 * @subsubsection TZSL Topaz TZSL
 * `tz_sampler2D_uniform(1) tex_sampler;`
 * 
 * Correctly resolves to either of the two outputs above, depending on the build config used to build the given TZSLC process.
 * <hr>
 * @subsection problem2 Problem: Different behaviour between OpenGL/Vulkan
 * Sometimes in shaders you may have functionality for a specific render-api but there is no common behaviour provided by TZSL. For example, Vulkan and OpenGL texture-coordinates are slightly different. You might want code which transforms the texture-coordinates to another format only if on Vulkan.
 * 
 * Unfortunately in GLSL there are no magic defines telling you which render-api will be building the shader.
 * @subsection solution2 Solution: Vulkan-only or OpenGL-only code fragments
 * Fortunately, when building with TZSLC such defines are injected into the code while transpiling to GLSL.
 * @subsubsection TZSL Topaz TZSL
 * ```c
 * layout(location = 0) in vec2 texcoord;
 * layout(location = 0) out vec4 output_colour;
 * 
 * void main()
 * {
 *      vec2 texcoord_transformed = texcoord;
 *      // Assume texcoord is in OpenGL format. If we're on Vulkan, we need to transform it or the image will be 90 degrees off.
 * 
 *      #if TZ_VULKAN
 *          texcoord_transformed.y = 1.0 - texcoord_transformed.y;
 *      #endif
 * 
 *      // Now identical output for both render-apis.
 *      output_colour = texture(tex_sampler, texcoord_transformed);
 *  }
 * ```
 * 
 */