#ifndef TOPAZ_TZSLC_STDLIB_HPP
#define TOPAZ_TZSLC_STDLIB_HPP
#include "textc/imported_text.hpp"
#include <string_view>
#include <span>

#include ImportedTextHeader(atomic, tzsl)
#include ImportedTextHeader(space, tzsl)
#include ImportedTextHeader(math, tzsl)
#include ImportedTextHeader(matrix, tzsl)
#include ImportedTextHeader(mesh, tzsl)
#include ImportedTextHeader(noise, tzsl)

namespace tzslc
{
	const std::string_view stdlib_atomic = ImportedTextData(atomic, tzsl);
	const std::string_view stdlib_space = ImportedTextData(space, tzsl);
	const std::string_view stdlib_noise = ImportedTextData(noise, tzsl);
	const std::string_view stdlib_math = ImportedTextData(math, tzsl);
	const std::string_view stdlib_matrix = ImportedTextData(matrix, tzsl);
	const std::string_view stdlib_mesh = ImportedTextData(mesh, tzsl);
}

#endif // TOPAZ_TZSLC_STDLIB_HPP
