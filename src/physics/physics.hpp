#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "physics/boundary.hpp"
#include "graphics/asset.hpp"

class StaticObject;
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
	AABB bound_aabb(const Mesh& mesh, const Matrix4x4& transform = Matrix4x4::identity());
    /**
     * Given a Model, creates a well-fitted axis-aligned-bounding-box around all vertices of all meshes in the model.
     * @param model - The model to bound
     * @return - AABB containing the model, in model-space if the model contained at least one mesh. If no meshes were contained, nullopt is returned
     */
	std::optional<AABB> bound_aabb(const Model& model, const Matrix4x4& transform = Matrix4x4::identity());
    /**
     * Given an asset, creates a well-fitted AABB around any meshes or models that the asset contains.
     * @param asset - The asset to bound
     * @return - AABB containing all vertices of any mesh/model in the asset. If none exist/contain no vertices, nullopt is returned
     */
    std::optional<AABB> bound_aabb(const Asset& asset, const Matrix4x4& transform = Matrix4x4::identity());
	std::optional<AABB> bound_object(const StaticObject& object);
	/**
	 * Given multiple Objects, creates a single well-fitted axis-aligned-bounding-box as tightly as possible around all the objects.
	 * Perfect to be used on the result of a tz::graphics::batch(...) call to bound the entirety of the batch.
	 */
	//AABB bound_aabb_batch(const std::vector<SceneObject>& objects);
}

std::ostream& operator<<(std::ostream& stream, const tz::physics::Axis2D& axis);
std::ostream& operator<<(std::ostream& stream, const tz::physics::Axis3D& axis);

#endif