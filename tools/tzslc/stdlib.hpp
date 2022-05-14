#ifndef TOPAZ_TZSLC_STDLIB_HPP
#define TOPAZ_TZSLC_STDLIB_HPP
#include "core/imported_text.hpp"
#include <string_view>
#include <span>

#include ImportedTextHeader(space, tzsl)
#include ImportedTextHeader(matrix, tzsl)

namespace tzslc
{
	const std::string_view stdlib_space = ImportedTextData(space, tzsl);
	const std::string_view stdlib_matrix = ImportedTextData(matrix, tzsl);
}

#endif // TOPAZ_TZSLC_STDLIB_HPP
