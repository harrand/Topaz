#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include "gl/vertex.hpp"
#include <vector>

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	struct Mesh
	{
		std::vector<tz::gl::Vertex> vertices;
	};

	struct IndexedMesh
	{
		std::vector<tz::gl::Vertex> vertices;
		std::vector<tz::gl::Index> indices;

		std::size_t data_size_bytes() const;
		std::size_t indices_size_bytes() const;
	};

	void sort_indices(IndexedMesh& mesh, tz::Vec3 closest_to);

	IndexedMesh screen_size_quad();

	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_MESH_HPP