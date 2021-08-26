/**
 * @page tzsl Topaz Shader Language
 * 
 * TZSL is a shading language virtually identical to GLSL. It is intended to be a clearer, easier-to-use variant of GLSL.
 * Unlike GLSL, TZSL has no differences across OpenGL/Vulkan. That is - there is no such thing as a Vulkan TZSL dialect.
 * 
 * Topaz expects shaders to be written in TZSL. The engine ships with a tool named TZSLC. See @ref tools for more information.
 * @section resources Resources
 * In Topaz, Renderers can have a shader attached to them aswell as resources. Unlike in GLSL, to refer to buffers/texture samplers one must refer to the corresponding resource within TZSL.
 * 
 * For example: If the @ref tz::gl::IRenderer has a texture resource with resource-id 1, then the TZSL to refer to that texture may be as follows:
 * ```c
 * resource(id = 1) const texture tex_sampler;
 * ```
 * Note the 'const'. This is extremely important in TZSL. A const texture is a normal texture sampler. A non-const texture is assumed to hold a valid image storage and writable from the shader.
 * 
 * The same holds true for buffers:
 * ```c
 * resource(id = 0) const buffer MVP
 * {
 *    mat4 m;
 *    mat4 v;
 *    mat4 p;
 * } mvp;
 * ```
 * As the buffer is marked const, it will end up being a UBO if the size permits (which it will in this particular case). This brings performance improvements.
 * Another example:
 * ```c
 * resource(id = 0) buffer MVP
 * {
 *    mat4 m;
 *    mat4 v;
 *    mat4 p;
 * } mvp;
 * ```
 * As this buffer is not const, it will end up as an SSBO. While these are correctly writeable, this will not perform as well than a const buffer resource if you don't actually intend to write to it.
 * @section example_vtx_shader Example Vertex Shader
 * ```c
 * #pragma shader_stage(vertex)
 *
 *    layout(location = 0) in vec3 position;
 *    layout(location = 1) in vec2 texcoord;
 *    layout(location = 2) in vec3 normal;
 *    layout(location = 3) in vec3 tangent;
 *    layout(location = 4) in vec3 bitangent;
 *
 *    resource(id = 0) const buffer MVP
 *    {
 *        mat4 m;
 *        mat4 v;
 *        mat4 p;
 *    } mvp;
 *
 *     layout(location = 0) out vec2 frag_tex_coord;
 *
 *    void main()
 *    {
 *            mat4 mvp_matrix = mvp.p * mvp.v * mvp.m;
 *        gl_Position = mvp_matrix * vec4(position, 1.0);
 *        frag_tex_coord = texcoord;
 *    }
 * ```
 */