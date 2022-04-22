/**
 * @page shaders Shaders
 * @section tzsl Topaz Shader Language
 * @section Introduction
 * TZSL is a high-level shading language with a syntax very similar to the OpenGL Shading Language (GLSL). While GLSL has slightly different dialects for different render APIs (such as OpenGL and Vulkan), TZSL shaders can be used for all render-API builds.
 *
 * TZSL has a bespoke syntax for specifying shader resources. This is because shader resources are concrete and first-class objects as far as topaz renderers are concerned.
 *
 * @section Language
 * @subsection Preprocessor Definitions
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
 * #pragma shader_stage(fragment)
 * layout(location = 0) out vec4 frag_colour;
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
 * @subsection Keywords
 * TZSL has special reserved keywords. Some keywords compile down directly into GLSL depending on the target render-api:
 * <table>
 * 	<tr>
 * 		<th>TZSL Keyword</th>
 * 		<th>OGL</th>
 * 		<th>VK</th>
 * 	</tr>
 * 	<tr>
 * 		<td>tz_VertexID</td>
 * 		<td>gl_VertexID</td>
 * 		<td>gl_VertexIndex</td>
 * 	</tr>
 * </table>
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
 * <hr>
 */
