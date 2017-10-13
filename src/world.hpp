#ifndef WORLD_HPP
#define WORLD_HPP
#include <map>
#include <cstddef>
#include "entity.hpp"
#include "light.hpp"
#include "mdl.hpp"

class World
{
public:
	World(std::string filename = "default.world", std::string resources_path = "resources.data");
	World(const World& copy);
	World(World&& move);
	~World();
	World& operator=(const World& rhs) = default;
		
	const std::string& get_file_name() const;
	const Vector3F& get_gravity() const;
	const Vector3F& get_spawn_point() const;
	const Vector3F& get_spawn_orientation() const;
	void set_gravity(Vector3F gravity = Vector3F());
	void set_spawn_point(Vector3F spawn_point = Vector3F());
	void set_spawn_orientation(Vector3F spawn_orientation = Vector3F());
	void add_object(Object obj);
	void add_entity(Entity ent);
	void add_entity_object(EntityObject eo);
	void add_light(Light light, GLuint shader_program_handle);
	void remove_object(Object obj);
	void remove_entity(Entity ent);
	void remove_entity_object(EntityObject eo);
	void remove_light(Light light);
	const std::vector<Object>& get_objects() const;
	const std::vector<Entity>& get_entities() const;
	const std::vector<EntityObject>& get_entity_objects() const;
	std::size_t get_size() const;
	const std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light>& get_lights() const;
	void kill_lights();
	void export_world(const std::string& world_link) const;
	void save() const;
	void render(Camera& cam, Shader& shader, unsigned int width, unsigned int height, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps);
	void update(unsigned int tps);
private:
	static Object retrieve_object_data(const std::string& object_name, std::string resources_path, MDLF& mdlf);
	static EntityObject retrieve_entity_object_data(const std::string& entity_object_name, std::string resources_path, MDLF& mdlf);
	static constexpr unsigned int MAXIMUM_LIGHTS = 8;
	std::string filename;
	std::string resources_path;
	Vector3F gravity, spawn_point, spawn_orientation;
	std::vector<Object> objects;
	std::vector<Entity> entities;
	std::vector<EntityObject> entity_objects;
	std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light> base_lights;
};
#endif