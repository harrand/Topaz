#ifndef TOPAZ_GL_IMPORTED_SHADERS_HPP
#define TOPAZ_GL_IMPORTED_SHADERS_HPP
#include <string_view>
#include <span>

#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X
#define CONCAT(X, Y) X##Y
#define JOIN(X, Y, Z) X##Y##Z

/**
 * @ingroup tz_gl2
 * @fn ImportedShaderHeader(shader_name, shader_type)
 * @hideinitializer
 * Retrieves a file path which is intended to be #included in a application's main source file. Once included, the imported shader's source code can be retrieved as a constexpr string_view via @ref ImportedShaderSource
 */
#define ImportedShaderHeader(shader_name, shader_type) STRINGIFY2(shader_name.shader_type.tzsl.hpp)

/**
 * @ingroup tz_gl2
 * @fn ImportedShaderSource(shader_name, shader_type)
 * @hideinitializer
 * Retrieves a token representing a `std::string_view` which contains the shader's compiled results. On Vulkan, this will be SPIRV, and GLSL source code for OpenGL. You can only retrieve an imported shader's source if the imported header has been included via @ref ImportedShaderHeader
 */
#define ImportedShaderSource(shader_name, shader_type) []()->std::string_view{std::span<const std::byte> shader_bin = std::as_bytes(std::span<const std::int8_t>(JOIN(shader_name, _, shader_type))); return std::string_view{reinterpret_cast<const char*>(shader_bin.data()), shader_bin.size_bytes()};}()

#endif // TOPAZ_GL_IMPORTED_SHADERS_HPP
