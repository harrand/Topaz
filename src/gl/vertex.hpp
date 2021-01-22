#ifndef TOPAZ_GL_VERTEX_HPP
#define TOPAZ_GL_VERTEX_HPP
#include "core/vector.hpp"

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
		tz::Vec3 normal = {0.0f, 0.0f, 0.0f};
		tz::Vec3 tangent = {0.0f, 0.0f, 0.0f};
		tz::Vec3 bitangent = {0.0f, 0.0f, 0.0f};
	};

	using Index = unsigned int;
	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_VERTEX_HPP