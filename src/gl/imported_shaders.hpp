#ifndef TOPAZ_GL_IMPORTED_SHADERS_HPP
#define TOPAZ_GL_IMPORTED_SHADERS_HPP
#include <string_view>
#include <span>

#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X

#define ImportedShaderFile(shader_name) STRINGIFY2(shader_name.tzsl.hpp)
#define GetImportedShaderSource(shader_name) []()->std::string_view{std::span<const std::byte> shader_bin = std::as_bytes(std::span<const std::int8_t>(shader_name)); return std::string_view{reinterpret_cast<const char*>(shader_bin.data()), shader_bin.size_bytes()};}()

#endif // TOPAZ_GL_IMPORTED_SHADERS_HPP
