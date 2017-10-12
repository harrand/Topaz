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

class Object
{
public:
	Object(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, Vector3F pos, Vector3F rot, Vector3F scale, unsigned int shininess = tz::graphics::default_shininess, float parallax_map_scale = tz::graphics::default_parallax_map_scale, float parallax_map_offset = tz::graphics::default_parallax_map_offset, float displacement_factor = tz::graphics::default_displacement_factor);
	Object(const Object& copy) = default;
	Object(Object&& move) = default;
	~Object() = default;
	Object& operator=(const Object& rhs) = default;
	
	const Vector3F& get_position() const;
	const Vector3F& get_rotation() const;
	const Vector3F& get_scale() const;
	unsigned int get_shininess() const;
	float get_parallax_map_scale() const;
	float get_parallax_map_offset() const;
	float get_displacement_factor() const;
	virtual void set_position(Vector3F position);
	void set_rotation(Vector3F rotation);
	void set_scale(Vector3F scale);
	void set_shininess(float shininess);
	void set_parallax_map_scale(float parallax_map_scale);
	void set_parallax_map_offset(float parallax_map_offset);
	void set_displacement_factor(float displacement_factor);
	const std::string& get_mesh_link() const;
	const std::vector<std::pair<std::string, Texture::TextureType>> get_textures() const;
	virtual void render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, DisplacementMap* dm, const Camera& cam, Shader& shad, float width, float height) const;
protected:
	Vector3F pos, rot, scale;
	unsigned int shininess;
	float parallax_map_scale, parallax_map_offset, displacement_factor;
	std::string mesh_link;
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
};

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