#ifndef TOPAZ_GL_VERTEX_HPP
#define TOPAZ_GL_VERTEX_HPP
#include "geo/vector.hpp"

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */
	struct Vertex
	{
		tz::Vec3 position;
		tz::Vec2 texcoord;
		tz::Vec3 normal;
		tz::Vec3 tangent;
		tz::Vec3 bitangent;
	};

	using Index = unsigned int;
	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_VERTEX_HPP