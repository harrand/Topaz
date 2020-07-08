#ifndef TOPAZ_GL_FORMAT_HPP
#define TOPAZ_GL_FORMAT_HPP
#include "glad/glad.h"
#include <cstddef>

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	/**
	 * Represents instructions on how to interpret Buffer data.
	 * 
	 * In OpenGL nomenclature, this can be considered a wrapper around glVertexAttribPointer.
	 */
	struct Format
	{
		/// Number of components (regardless of whether components are ints, floats, etc...).
		std::size_t num_components;
		/// OpenGL enum type corresponding to the type of each component (such as GL_FLOAT).
		GLenum component_type;
		/// Size of one component type, in bytes.
		std::size_t component_size;
		/// Offset, in bytes, that the data starts from the beginning of the buffer.
		std::ptrdiff_t offset;
	};

	namespace fmt
	{
		/// Pre-defined format comprised of a trio of three floats, starting from the beginning of the buffer data.
		constexpr Format three_floats = Format{3, GL_FLOAT, sizeof(float), 0};
		constexpr Format two_floats = Format{2, GL_FLOAT, sizeof(float), 0};
	}

	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_FORMAT_HPP