#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "mesh.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "physics/boundary.hpp"
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
	Object2D(Vector2F position, float rotation, Vector2F scale, Vector4F colour = Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
	Object2D& operator=(const Object2D& copy);
	
	virtual void render(const Camera& cam, Shader* shader, float width, float height) const;
	
	Vector2F position, scale;
	float rotation;
	Vector4F colour;
protected:
	Mesh quad;
};

/**
* Collaboration of a mesh, texture, normal-map, parallax-map and displacement-map
* Use this to represent a 3D object, including its vertex data, texture, material etc.
*/

class Object
{
public:
	Object(std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh, Material material, Vector3F position, Vector3F rotation, Vector3F scale);
	Object(const Object& copy) = default;
	Object(Object&& move) = default;
	~Object() = default;
	Object& operator=(const Object& rhs) = default;
	
	const Mesh& get_mesh() const;
	//const std::map<tz::graphics::TextureType, Texture*>& get_textures() const;
	const Material& get_material() const;
	/**
	* Complexity: O(n) Ω(1) ϴ(n), where n =~ size of mesh data (will not return until GPU finishes processing)
	*/
	virtual void render(const Camera& cam, Shader* shader, float width, float height) const;
	bool operator==(const Object& rhs) const;
	
	Vector3F position, rotation, scale;
protected:
	std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh;
	Material material;
};

/**
* Wraps an OpenGL cubemap via a set of six textures.
* Use this to render skyboxes in a 3D world easily. Bring your own skybox shader though (Default one provided with Topaz is called 'skybox').
*/
class Skybox
{
public:
	Skybox(std::string cube_mesh_link, CubeMap& cm);
	Skybox(const Skybox& copy) = default;
	Skybox(Skybox&& move) = default;
	~Skybox() = default;
	Skybox& operator=(const Skybox& rhs) = default;
	void render(const Camera& cam, Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height);
private:
	std::string cube_mesh_link;
	CubeMap& cm;
};

namespace tz::graphics
{
	/**
	* Creates a new Object3D with InstancedMesh containing all data from the objects parameter.
	* All elements of 'objects' should share the same texture and mesh.
	* This function performs batching; allowing multiple objects to be rendered with a single draw call, serving as a gargantuan optimisation under the right conditions.
	*/
	template <template <typename> class Collection>
	Object batch(const Collection<Object>& objects);
	/**
	* Creates multiple Objects using batch (see above).
	* However, the returned map has the first element in which the Object3D value shares textures and meshes.
	* This can be used on a World object list to batch ALL elements, organising which objects share the same asset data.
	*/
	template <template <typename> class Collection>
	std::vector<Object> batch_full(const Collection<Object>& objects);
}

#include "object.inl"
#endif