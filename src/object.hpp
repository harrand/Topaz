#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "boundary.hpp"
#include <variant>

namespace tz::graphics
{
	constexpr unsigned int default_shininess = 5;
	constexpr float default_parallax_map_scale = 0.04f;
	constexpr float default_parallax_map_offset = -0.5f;
	constexpr float default_displacement_factor = 0.25f;
}

/*
	Collaboration of a mesh, texture, normal-map, parallax-map and displacement-map. Use this to represent a 3D object completely, including its vertex data, texture, material etc.
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
	// Complexity: O(n) Ω(1) ϴ(n), where n =~ size of mesh data (will not return until GPU finishes processing)
	virtual void render(const Camera& cam, Shader* shader, float width, float height);
	bool operator==(const Object& rhs) const;
	
	Vector3F position, rotation, scale;
	unsigned int shininess;
	float parallax_map_scale, parallax_map_offset, displacement_factor;
protected:
	std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh;
	std::map<tz::graphics::TextureType, Texture*> textures;
};

/*
	Wraps an OpenGL cubemap via a set of six textures. Use this to render skyboxes in a 3D world easily. Bring your own skybox shader though (Default one provided with Topaz is called 'skybox').
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
	/*
		Creates a new Object with InstancedMesh containing all data from the objects parameter.
		All elements of 'objects' should share the same texture and mesh.
		This function performs instancification; allowing multiple objects to be rendered with a single draw call, serving as a gargantuan optimisation.
	*/
	Object instancify(const std::vector<Object>& objects);
}

#endif