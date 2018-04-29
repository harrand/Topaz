#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "mesh.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "../camera.hpp"
#include "../physics/physics.hpp"
#include <variant>

namespace tz::graphics
{
	constexpr unsigned int default_shininess = 5;
	constexpr float default_parallax_map_scale = 0.04f;
	constexpr float default_parallax_map_offset = -0.5f;
	constexpr float default_displacement_factor = 0.25f;
}

/**
* Simple plane mesh with a colour to represent a simple-sprite.
*/
class Object2D
{
public:
	/**
	 * Construct an Object2D from all specifications.
	 * @param position - 2-dimensional Vector representing the position of the Object2D, in world-space
	 * @param rotation - Scalar representing the orientation of the Object2D, in radians
	 * @param scale - 2-dimensional Vector representing the scale of the Object2D in each spatial dimension
	 * @param colour - RGBA-encoded colour of the Object's mesh
	 */
	Object2D(Vector2F position, float rotation, Vector2F scale, Vector4F colour = Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
	/**
	 * Assign this Object2D to be a copy of another.
	 * @param copy - The Object2D for this to become a copy of
	 * @return - This, after assignment
	 */
	Object2D& operator=(const Object2D& copy);

	/**
	 * Render this Object2D, through an existing Camera via an existing Shader
	 * @param cam - The Camera through which to view the Object2D
	 * @param shader - The Shader with which to render the Object2D
	 * @param width - The width of the viewport, in any unit proportional to pixels
	 * @param height - The height of the viewport, in any unit proportional to pixels
	 */
	virtual void render(const Camera& cam, Shader* shader, float width, float height) const;

	/// Position of the Object2D, in screen-space.
	Vector2F position;
	/// Scale of the Object2D, in pixels.
	Vector2F scale;
	/// Rotation of the Object2D, in radians.
	float rotation;
	/// RGBA-encoded colour of the Object2D.
	Vector4F colour;
protected:
	/// Underlying quad Mesh.
	Mesh quad;
};

/**
* Collaboration of a mesh, texture, normal-map, parallax-map and displacement-map
* Use this to represent a 3D object, including its vertex data, texture, material etc.
*/
class Object
{
public:
	/**
	 * Construct an Object with all specifications.
	 * @param mesh - Either an owning or non-owning pointer to the Mesh which this Object should use
	 * @param material - The Material that this Object should use
	 * @param position - 3-dimensional Vector representing the position of the Object, in world-space
	 * @param rotation - 3-dimensional Vector representing the orientation of the Object, in euler-angles
	 * @param scale - 3-dimensional Vector representing the scale of the Object, in each spatial dimension
	 */
	Object(std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh, Material material, Vector3F position, Vector3F rotation, Vector3F scale);

	/**
	 * Read-only access to the underlying Mesh.
	 * @return - The Mesh that this Object is using
	 */
	const Mesh& get_mesh() const;
	/**
	 * Read-only access to the underlying Material.
	 * @return - The Material that this Object is using
	 */
	const Material& get_material() const;
	/**
	 * Render this Object through the specified Camera via the specified Shader.
	 * Complexity: O(n) Ω(1) ϴ(n), where n =~ size of mesh data (will not return until GPU finishes processing)
	 * @param cam - The Camera through which to view the Object
	 * @param shader - The Shader with which to render the Object
	 * @param width - Width of the viewport, in any unit proportional to pixels
	 * @param height - Height of the viewport, in any unit proportional to pixels
	 */
	virtual void render(const Camera& cam, Shader* shader, float width, float height) const;
	/**
	 * Equate this Object with another
	 * @param rhs - The other Object
	 * @return - True if the Objects are equal. Otherwise false
	 */
	bool operator==(const Object& rhs) const;

	/// Position of the Object in world-space
	Vector3F position;
	/// Orientation of the Object in euler-angles
	Vector3F rotation;
	/// Scale of the Object in each spatial dimension
	Vector3F scale;
protected:
	/// Underlying (non)owning pointer to the Mesh being used.
	std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh;
	/// Material used by the Object.
	Material material;
	/// Bounding Box, in model-space.
	AABB boundary_modelspace;
};

/**
* Wraps an OpenGL cubemap via a set of six textures.
* Use this to render skyboxes in a 3D world easily. Bring your own skybox shader though (Default one provided with Topaz is called 'skybox').
*/
class Skybox
{
public:
	/**
	 * Construct a Skybox with all specifications.
	 * @param cube_mesh_link - Path to the cube-map Mesh being used
	 * @param cm - Existing CubeMap to utilise for this Skybox
	 */
	Skybox(std::string cube_mesh_link, CubeMap& cm);
	/**
	 * Render this Skybox, given the container of Mesh assets containing the Skybox
	 * @param cam - Camera through which to view the Skybox
	 * @param shad - Shader with which to render the Skybox
	 * @param all_meshes - Container of Mesh assets
	 * @param width - Width of the viewport, in any unit proportional to pixels
	 * @param height - Height of the viewport, in any unit proportional to pixels
	 */
	void render(const Camera& cam, Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height);
private:
	/// Path to the cube-map Mesh being used by this Skybox.
	std::string cube_mesh_link;
	/// Underlying CubeMap.
	CubeMap& cm;
};

namespace tz::graphics
{
	/**
	 * Creates a new Object3D with InstancedMesh containing all data from the objects parameter.
	 * All elements of 'objects' should share the same texture and mesh.
	 * This function performs batching; allowing multiple objects to be rendered with a single draw call, serving as a gargantuan optimisation under the right conditions.
	 * @tparam Collection - Type of container holding the Objects. Must support range-based for-loop
	 * @param objects - Container of all the Objects to batch together
	 * @return - The singular batched Object
	 */
	template <template <typename> class Collection>
	Object batch(const Collection<Object>& objects);
	/**
	 * Creates multiple Objects using batch (see above).
	 * However, the returned map has the first element in which the Object3D value shares textures and meshes.
	 * This can be used on a World object list to batch ALL elements, organising which objects share the same asset data.
	 * @tparam Collection - Type of container holding the Objects. Must support range-based for-loop
	 * @param objects - Container of all the Objects to batch together
	 * @return - The smallest possible container of batched Objects
	 */
	template <template <typename> class Collection>
	std::vector<Object> batch_full(const Collection<Object>& objects);
}

#include "object.inl"
#endif