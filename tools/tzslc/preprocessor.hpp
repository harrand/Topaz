#ifndef TZSLC_PREPROCESSOR_HPP
#define TZSLC_PREPROCESSOR_HPP
#include "core/containers/enum_field.hpp"
#include <array>
#include <string>

namespace tzslc
{
	bool preprocess(std::string& shader_source);
	bool preprocess_samplers(std::string& shader_source);
	bool preprocess_prints(std::string& shader_source);
	bool preprocess_asserts(std::string& shader_source);
	bool preprocess_topaz_types(std::string& shader_source);
}

#endif // TZSLC_PREPROCESSOR_HPP
