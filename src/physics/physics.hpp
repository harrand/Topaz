#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "physics/boundary.hpp"
#include "graphics/mesh.hpp"

/// Pre-define the Object class.
class Object;

namespace tz::physics
{
	enum class Axis2D : unsigned int
	{
		X = 0, Y = 1
	};

	enum class Axis3D : unsigned int
	{
		X = 0, Y = 1, Z = 2
	};

	/**
	* Given a Mesh, creates a well-fitted sphere-boundary around the vertices of a mesh.
	* The BoundingSphere is bound in model-space.
	*/
	BoundingSphere bound_sphere(const Mesh& mesh);
	/**
	 * Given a Mesh, creates a well-fitted axis-aligned-bounding-box around the vertices of a mesh.
	 * The AABB is bound in model-space.
	 */
	AABB bound_aabb(const Mesh& mesh);
	/**
	 * Given multiple Objects, creates a single well-fitted axis-aligned-bounding-box as tightly as possible around all the objects.
	 * Perfect to be used on the result of a tz::graphics::batch(...) call to bound the entirety of the batch.
	 */
	//AABB bound_aabb_batch(const std::vector<SceneObject>& objects);
}

std::ostream& operator<<(std::ostream& stream, const tz::physics::Axis2D& axis);
std::ostream& operator<<(std::ostream& stream, const tz::physics::Axis3D& axis);

#endif