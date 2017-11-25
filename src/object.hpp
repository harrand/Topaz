#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "boundary.hpp"
#include <initializer_list>

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
	Object(std::string mesh_link, std::map<Texture::TextureType, std::string> textures, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess = tz::graphics::default_shininess, float parallax_map_scale = tz::graphics::default_parallax_map_scale, float parallax_map_offset = tz::graphics::default_parallax_map_offset, float displacement_factor = tz::graphics::default_displacement_factor);
	Object(const Object& copy) = default;
	Object(Object&& move) = default;
	~Object() = default;
	Object& operator=(const Object& rhs) = default;
	
	const std::string& get_mesh_link() const;
	const std::map<Texture::TextureType, std::string> get_textures() const;
	virtual void render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, DisplacementMap* dm, const Camera& cam, Shader& shad, float width, float height) const;
	virtual void render(const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps, const Camera& cam, Shader& shad, float width, float height) const;
	bool operator==(const Object& rhs) const;
	
	Vector3F position, rotation, scale;
	unsigned int shininess;
	float parallax_map_scale, parallax_map_offset, displacement_factor;
protected:
	std::string mesh_link;
	std::map<Texture::TextureType, std::string> textures;
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

#endif