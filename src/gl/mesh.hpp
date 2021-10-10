#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include "core/vector.hpp"
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include "gl/renderer.hpp"

namespace tz::gl
{
	/**
	 * @brief Represents the typical vertex type Topaz expects. This is the data type used by tz::gl::Mesh and tz::gl::MeshInput.
	 */
	struct Vertex
	{
		tz::Vec3 position = {};
		tz::Vec2 texcoord = {};
		tz::Vec3 normal = {};
		tz::Vec3 tangent = {};
		tz::Vec3 bitangent = {};
	};

	/**
	 * @brief Represents the typical mesh type Topaz expects.
	 * 
	 */
	struct Mesh
	{
		tz::BasicList<Vertex> vertices;
		tz::BasicList<unsigned int> indices;
	};
}

#endif // TOPAZ_GL_MESH_HPP