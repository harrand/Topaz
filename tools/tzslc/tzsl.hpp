#ifndef TZSLC_PREPROCESSOR_HPP
#define TZSLC_PREPROCESSOR_HPP
#include "core/containers/enum_field.hpp"
#include <array>
#include <string>
#include <filesystem>

namespace tzslc
{
	void compile_to_glsl(std::string& shader_source, std::filesystem::path shader_filename);
}

#endif // TZSLC_PREPROCESSOR_HPP
