#ifndef TOPAZ_GL_MESH_LOADER_HPP
#define TOPAZ_GL_MESH_LOADER_HPP
#include "gl/mesh.hpp"
#include "gl/tz_assimp/scene.hpp"

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	tz::gl::IndexedMesh load_mesh(const std::string& filename);

	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_MESH_LOADER_HPP