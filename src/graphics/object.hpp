#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
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
* Like a normal Object2D, but has a texture bound.
*/
class Sprite : public Object2D
{
public:
	Sprite(Vector2F position, float rotation, Vector2F scale, Texture* texture);
	Sprite& operator=(const Sprite& copy);
	virtual void render(const Camera& cam, Shader* shader, float width, float height) const override;
private:
	Texture* texture;
};

/**
* Collaboration of a mesh, texture, normal-map, parallax-map and displacement-map
* Use this to represent a 3D object, including its vertex data, texture, material etc.
*/

class Object
{
public:
	Object(std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh, std::map<tz::graphics::TextureType, Texture*> textures, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess = tz::graphics::default_shininess, float parallax_map_scale = tz::graphics::default_parallax_map_scale, float parallax_map_offset = tz::graphics::default_parallax_map_offset, float displacement_factor = tz::graphics::default_displacement_factor);
	Object(const Object& copy) = default;
	Object(Object&& move) = default;
	~Object() = default;
	Object& operator=(const Object& rhs) = default;
	
	const Mesh& get_mesh() const;
	const std::map<tz::graphics::TextureType, Texture*>& get_textures() const;
	/**
	* Complexity: O(n) Ω(1) ϴ(n), where n =~ size of mesh data (will not return until GPU finishes processing)
	*/
	virtual void render(const Camera& cam, Shader* shader, float width, float height) const;
	bool operator==(const Object& rhs) const;
	
	Vector3F position, rotation, scale;
	unsigned int shininess;
	float parallax_map_scale, parallax_map_offset, displacement_factor;
protected:
	std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh;
	std::map<tz::graphics::TextureType, Texture*> textures;
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
	* This function performs instancification; allowing multiple objects to be rendered with a single draw call, serving as a gargantuan optimisation under the right conditions.
	*/
	template <template <typename> class Collection>
	Object instancify(const Collection<Object>& objects);
	/**
	* Creates multiple Objects using instancify (see above).
	* However, the returned map has the first element in which the Object3D value shares textures and meshes.
	* This can be used on a World object list to instancify ALL elements, organising which objects share the same asset data.
	*/
	template <template <typename> class Collection>
	std::vector<Object> instancify_full(const Collection<Object>& objects);
}

#include "object.inl"
#endif