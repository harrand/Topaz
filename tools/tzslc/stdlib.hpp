#ifndef TOPAZ_TZSLC_STDLIB_HPP
#define TOPAZ_TZSLC_STDLIB_HPP
#include <string_view>
#include <span>

#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X
#define CONCAT(X, Y) X##Y
#define JOIN(X, Y, Z) X##Y##Z

#define ImportedTextHeader(text_name, text_ext) STRINGIFY2(text_name.text_ext.hpp)

/**
 * @ingroup tz_gl2
 * @fn ImportedShaderSource(shader_name, shader_type)
 * @hideinitializer
 * Retrieves a token representing a `std::string_view` which contains the shader's compiled results. On Vulkan, this will be SPIRV, and GLSL source code for OpenGL. You can only retrieve an imported shader's source if the imported header has been included via @ref ImportedShaderHeader
 */
#define ImportedTextData(text_name, text_ext) []()->std::string_view{std::span<const std::byte> shader_bin = std::as_bytes(std::span<const std::int8_t>(JOIN(text_name, _, text_ext))); return std::string_view{reinterpret_cast<const char*>(shader_bin.data()), shader_bin.size_bytes()};}()

#include ImportedTextHeader(space, tzsl)
#include ImportedTextHeader(matrix, tzsl)

namespace tzslc
{
	const std::string_view stdlib_space = ImportedTextData(space, tzsl);
	const std::string_view stdlib_matrix = ImportedTextData(matrix, tzsl);
}

#endif // TOPAZ_TZSLC_STDLIB_HPP
