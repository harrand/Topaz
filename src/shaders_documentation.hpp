/**
 * @page shaders Shaders
 * @section tzsl Topaz Shader Language
 * @section Introduction
 * TZSL is a high-level shader language with a syntax very similar to that of GLSL. A tool named tzslc ships with the engine, which can be used to build tzsl shaders to output SPIRV, GLSL shaders or SPIRV encoded within a C++ header.
 *
 * TZSL Shaders come in three types:
 * - Vertex Shaders, which specify the locations of individual vertices.
 * 	- There are no vertex buffers, so vertex-pulling is a must.
 * 	- Index buffers are planned, but not yet implemented. For the time being you must perform index-pulling aswell.
 * - Fragment Shaders, which processes fragments into a set of colours and optionally depth value.
 * - Compute Shaders, which can be used to perform general-purpose computation on the GPU.
 *
 * TZSL Shaders have first-class support in the engine's build system. When specifying your application in CMake, you can specify tzsl shader files. When the application needs to be built, the shaders are compiled into specially-encoded C++ headers which you can include in your application at compile-time. This is known as a TZSL Header Import.
 *
 * @section Language
 * @subsection Inputs
 * Fragment shaders may have inputs. These are variable passed from the output of a vertex shader. Because Topaz does not deal with input state (there are no vertex buffers), vertex shaders cannot have inputs. Compute shaders cannot have inputs either, but that is normal. Every input must have an index, type and name.
 *
 *
 * Syntax:
 * `input(id = x) variable_type variable_name;`
 * Example:
 * `input(id = 0) vec2 texcoord;`
 * @subsection Outputs
 * Vertex and fragment shaders may have any number of outputs. These are similar to `out` variables from GLSL, although the syntax is slightly different. Every output must have an index, type and name.
 *
 * Syntax:
 * `output(id = x) variable_type variable_name;`
 * Example:
 * `output(id = 0) vec3 frag_colour;`
 * @subsection Resources
 * When writing a shader, you will know about the shader resources it has access to. You might be used to the following syntaxes in OpenGL:
 * ```c
 * uniform sampler2D my_image;
 * layout(binding = 0) uniform MyUniformBuffer
 * {
 *	...
 * } buf;
 * ```
 * Note that the syntax for specifying image and buffer resources are quite different. Because image and buffer resources are first-class objects as far as Topaz renderers are concerned, the syntax is unified and obvious:
 * ```c
 * resource(id = 0) const texture my_image;
 * resource(id = 0) const buffer MyUniformBuffer
 * {
 * 	...
 * } buf;
 * ```
 * Note that specifying resource ids for shader resources is mandatory, regardless of resource type. The resource id is constant and corresponds to the @ref tz::gl::ResourceHandle when being specified to a renderer. This means that resources have unique identifiers. That is, if a vertex shader specifies an image resource at id 0, and again in a fragment shader (within the same shader program), they both refer to the same resource.
 *
 * Note: The example above is actually ill-formed -- An image and buffer resource are specified, but they share the same id which is impossible; a resource is either a buffer or an image resource, never both.
 * @subsection preproc Preprocessor Definitions
 * A number of preprocessor definitions are guaranteed to be exist for all TZSL shaders. These are always defined, but their value depends on the build parameters of the target program.
 *
 * Below are the defines whose value depends on the render-api:
 * <table>
 * 	<tr>
 * 		<th>Define</th>
 * 		<th>Value (OpenGL)</th>
 * 		<th>Value (Vulkan)</th>
 * 	</tr>
 * 	<tr>
 *		<td>TZ_VULKAN</td>
 *		<td style="text-align: center;">0</td>
 *		<td style="text-align: center;">1</td>
 * 	</tr>
 * 	<tr>
 *		<td>TZ_OGL</td>
 *		<td style="text-align: center;">1</td>
 *		<td style="text-align: center;">0</td>
 * 	</tr>
 * </table>
 *
 * Below are the defines whose value depends on the build-config:
 * <table>
 * 	<tr>
 * 		<th>Define</th>
 * 		<th>Value (Debug)</th>
 * 		<th>Value (Release)</th>
 * 	</tr>
 * 	<tr>
 *		<td>TZ_DEBUG</td>
 *		<td style="text-align: center;">1</td>
 *		<td style="text-align: center;">0</td>
 * 	</tr>
 * </table>
 * Just for clarity, these are preprocessor definitions as in GLSL or C. These are useful if you want your shader to act differently on certain build configurations, but without having to write different shaders.
 *
 * @subsubsection ex0 Example Fragment Shader
 * ```c
 * shader(type = fragment);
 * output(id = 0) vec4 frag_colour;
 *
 * void main()
 * {
 * 	// Output is a solid colour. Red if we're on Vulkan, otherwise white.
 *	#if TZ_VULKAN
 *		frag_colour = vec4(1.0, 0.0, 0.0, 1.0);
 *	#else
 *		frag_colour = vec4(1.0, 1.0, 1.0, 1.0);
 *	#endif
 * }
 * ```
 * @section stypes IO Blocks
 * Below are the input and output blocks for each programmable shader type. Each shader stage has a fixed-set of input and output variables, in the `in::` and `out::` namespaces respectively. These are not the same as input and output language specifiers.
 * <details>
 * <summary>Vertex Shader</summary>
 * <table>
 * 	<tr>
 * 		<th>Variable</th>
 * 		<th>GLSL Equivalent</th>
 * 	</tr>
 * 	<tr>
 *		<td>in::vertex_id</td>
 *		<td>gl_VertexID</td>
 * 	</tr>
 * 	<tr>
 *		<td>in::instance_id</td>
 *		<td>gl_InstanceID</td>
 * 	</tr>
 * 	<tr>
 *		<td>out::position</td>
 *		<td>gl_Position</td>
 * 	</tr>
 * </table>
 * </details>
 *
 * <details>
 * <summary>Fragment Shader</summary>
 * <table>
 * 	<tr>
 * 		<th>Variable</th>
 * 		<th>GLSL Equivalent</th>
 * 	</tr>
 * 	<tr>
 *		<td>in::fragment_coord</td>
 *		<td>gl_FragCoord</td>
 * 	</tr>
 * 	<tr>
 *		<td>out::fragment_depth</td>
 *		<td>gl_FragDepth</td>
 * 	</tr>
 * </table>
 * </details>
 *
 * <details>
 * <summary>Compute Shader</summary>
 * <table>
 * 	<tr>
 * 		<th>Variable</th>
 * 		<th>GLSL Equivalent</th>
 * 	</tr>
 * 	<tr>
 *		<td>in::workgroup_count</td>
 *		<td>gl_NumWorkGroups</td>
 * 	</tr>
 * 	<tr>
 *		<td>in::workgroup_id</td>
 *		<td>gl_WorkGroupID</td>
 * 	</tr>
 * 	<tr>
 *		<td>in::local_id</td>
 *		<td>gl_LocalInvocationID</td>
 * 	</tr>
 * 	<tr>
 *		<td>in::global_id</td>
 *		<td>gl_GlobalInvocationID</td>
 * 	</tr>
 * </table>
 * </details>
 *
 * @section bif Built-in Functions
 * @subsection bif_printf tz_printf
 * `void tz_printf(fmt: string-literal, ...)`
 * @note This is only available for Vulkan Debug builds. Otherwise, compiles down to no-op.
 *
 * Specifies a debug-printf statement. The syntax is c-like. Intuitively you might expect the printed output to be sent to the host program's stdout. This is not the case. At present, you can only view these in graphics debuggers which support the vulkan `debugPrintfEXT`, namely RenderDoc.
 *
 * The frequency of this output logically matches that of the context. If for example you're invoking `tz_printf` in a vertex shader's main function, a message will be emitted for each vertex drawn.
 *
 * To view the output message in RenderDoc:
 * <ol>
 *	<li>Capture a frame using a renderer that uses a shader with this debug output. Ensure the host program is Vulkan Debug.</li>
 *	<li>In the 'Event Browser', navigate to the draw call that invokes the shader.</li>
 *	<li>You should see 'X msg(s)' annotated on the draw call. Click on this to see each printf invocation.</li>
 * </ol>
 * @subsubsection tz_printf_params Parameters
 * The parameters are much like C `printf`:
 * - String-literal specifying how to interpret the data. Regarding scalar types, format modifiers match that of C's `printf` function. However, format modifiers also exist for the built-in GLSL vector types.
 * 	- Format for specifier is "%"precision <d, i, o, u, x, X, a, A, e, E, f, F, g, G, ul, lu, lx>
 * 	- Format for vector specifier is "%"precision"v" [2, 3, 4] [specifiers above]
 * - Arguments specifying data to print. There must be an entry for each format modifier within the format string. Like `printf`, if no modifiers were specified in the format string, this parameter can be omitted.
 * @subsubsection debug_printf_example Examples
 * Here are a few example usages:
 * ```c
 * float myfloat = 3.1415f;
 * vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
 * uint64_t bigvar = 0x2000000000000001ul;
 *
 * tz_printf("Here's a float value with full precision %f", myfloat);
 * tz_printf("Here's a float value to 2 decimal places %1.2f", myfloat);
 * tz_printf("Here's a vector of floats %1.2v4f", floatvec);
 * tz_printf("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
 * ```
 * Outputs:
 * ```diff
 * Here's a float value with full precision 3.141500
 * Here's a float value to 2 decimal places 3.14
 * Here's a vector of floats 1.20, 2.20, 3.20, 4.20
 * Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001
 * ```
 *
 * @section header_imports TZSL Header Imports
 * Note that this feature is optional. If you don't like it, you can completely ignore it.
 *
 *
 * Say you have a project structure similar to the file tree below:
 *
 * <table>
 * 	<tr>
 * 		<th>Projects/MyGame</th>
 * 	</tr>
 * 	<tr>
 * 		<td>CMakeLists.txt</td>
 * 	</tr>
 * 	<tr>
 * 		<td>main.cpp</td>
 * 	</tr>
 * 	<tr>
 * 		<td>my_shader.vertex.tzsl</td>
 * 	</tr>
 * 	<tr>
 * 		<td>my_shader.fragment.tzsl</td>
 * 	</tr>
 * </table>
 *
 * Using a header import, you can access the shaders at compile-time within main.cpp.
 * @subsubsection cml_example CMakeLists.txt
 * ```cmake
 * add_app(
 * 	TARGET MyGame
 * 	SOURCE_FILES main.cpp
 * 	SHADER_SOURCES
 * 		my_shader.vertex.tzsl
 * 		my_shader.fragment.tzsl
 * )
 * ```
 *
 * @subsubsection maincpp main.cpp
 * ```cpp
 * #include "gl/imported_shaders.hpp"
 * #include <string_view>
 *
 * // Includes the compiled tzsl c++ header within this translation unit.
 * // NOTE: This is only doable if you specified the shader files within `add_application` in the build system.
 * #include ImportedShaderHeader(my_shader, vertex)
 * #include ImportedShaderHeader(my_shader, fragment)
 * int main()
 * {
 * 	constexpr std::string_view vtx_src = ImportedShaderSource(my_shader, vertex);
 * 	constexpr std::string_view frg_src = ImportedShaderSource(my_shader, fragment);
 *	// Note: The contents of the string depend on the current render-api. However, this is guaranteed to be compatible with tz::gl::ShaderInfo::set_shader(...).
 *	// For the curious: In Vulkan, the string contains SPIRV. In OpenGL, the string contains GLSL source.
 * }
 * ```
 *
 *
 * <hr>
 * @section rwe Example Shader: tz_dynamic_triangle_demo.fragment.tzsl
 * The following is the fragment shader used by `tz_dynamic_triangle_demo`:
 * @include gl/tz_dynamic_triangle_demo.fragment.tzsl
 *
 * @subsection rwe_analysis Analysis
 * - The shader contains a type declaration at the top, like all shaders. This informs tzslc that this is a fragment shader.
 * - The shader uses a single resource, an array of samplers with index 1.
 * - The shader has one input and one output, both with index 0.
 * 	- As you might expect, output 0 goes to the colour attachment 0, and input 0 is the first output from the vertex shader.
 */
