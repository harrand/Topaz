#ifndef TOPAZ_GL_FORMAT_HPP
#define TOPAZ_GL_FORMAT_HPP
#include "glad/glad.h"
#include <cstddef>

namespace tz::gl
{
	struct Format
	{
		std::size_t num_components;
		GLenum component_type;
		std::size_t component_size;
		std::ptrdiff_t offset;
	};

	namespace fmt
	{
		constexpr Format three_floats = Format{3, GL_FLOAT, sizeof(float), 0};
	}
}

#endif // TOPAZ_GL_FORMAT_HPP