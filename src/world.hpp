#ifndef WORLD_HPP
#define WORLD_HPP
#include <map>
#include <cstddef>
#include "entity.hpp"
#include "light.hpp"
#include "MDL/mdl.hpp"

/*
	Contains Objects, EntityObjects and pretty much any Topaz renderable you can think of. Handles all their physics inputs aswell. Takes a filename in its constructor for the MDL data file containing world data. Use this to store everything you want to draw.
*/
class World
{
public:
	// Engine cannot initialise World in its initialiser-list, so a default world must be available. It needn't serve any functionality, however. Unfortunately one of the unavoidable drawbacks of C++.
	World(std::string filename = "default.world", std::string resources_path = "resources.mdl", const std::vector<std::unique_ptr<Mesh>>& all_meshes = std::vector<std::unique_ptr<Mesh>>(), const std::vector<std::unique_ptr<Texture>>& all_textures = std::vector<std::unique_ptr<Texture>>(), const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps = std::vector<std::unique_ptr<NormalMap>>(), const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps = std::vector<std::unique_ptr<ParallaxMap>>(), const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps = std::vector<std::unique_ptr<DisplacementMap>>());
	World(const World& copy);
	World(World&& move);
	~World() = default;
	World& operator=(const World& rhs) = default;

	const std::string& get_file_name() const;
	const Vector3F& get_gravity() const;
	// Complexity: O(n*f_n + m*f_m) Ω(n*f_n + m*f_m) ϴ(1) where n = number of entities, f_n = number of forces per entity, m = number of entity_objects, f_m = number of forces per entity_object
	void set_gravity(Vector3F gravity = Vector3F());
	// Complexity: O(1) amortised Ω(1) ϴ(1) amortised
	void add_object(Object obj);
	// Complexity: O(n) Ω(1) ϴ(n), where n = number of existing entity_objects.
	void add_entity(Entity ent);
	// Complexity: See World::add_entity.
	void add_entity_object(EntityObject eo);
	// Complexity: O(n log n) Ω(log n) ϴ(log n), where n = number of existing lights.
	void add_light(Light light, GLuint shader_program_handle);
	template<class Element, typename... Args>
	Element& emplace(Args&&... args);
	template<typename... Args>
	Object& emplace_object(Args&&... args);
	template<typename... Args>
	Entity& emplace_entity(Args&&... args);
	template<typename... Args>
	EntityObject& emplace_entity_object(Args&&... args);
	void remove_object(const Object& obj);
	void remove_entity(const Entity& ent);
	void remove_entity_object(const EntityObject& eo);
	void remove_light(const Light& light);
	const std::vector<Object>& get_objects() const;
	const std::vector<Entity>& get_entities() const;
	const std::vector<EntityObject>& get_entity_objects() const;
	// Returns total number of Objects, Entities and EntityObjects in the world.
	std::size_t get_size() const;
	const std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light>& get_lights() const;
	// Updates uniforms to currently bound shader, zeroing light values for all lights. Complexity: O(n) Ω(1) ϴ(n), where n = number of lights.
	void kill_lights();
	// Export world data to a MDL file called world_link. Complexity: O(n + m) Ω(1) ϴ(n + m), where n = number of objects and m = number of entity_objects.
	void export_world(const std::string& world_link) const;
	// Export world data to a MDL file with the same name as the file which loaded this world, overwriting it. Complexity: See World::export_world.
	void save() const;
	// Render all elements in the world from the perspective of the camera, attaching a shader and updating uniforms in the process. Width and height parameters required to generate projection matrices & correct aspect-ratio. This method should be invoked as often as possible, to smooth gameplay. Complexity: O(n + p + o) Ω(1) ϴ(n + p + o), where n = number of objects, p = number of entity_objects, o = number of lights.
	void render(const Camera& cam, Shader* shader, unsigned int width, unsigned int height);
	// Update all elements in the world that obey some form of law of physics. Pass tps as the expected ticks-per-second, not the instantaneous tick per second. This function should be called per 'tick'. Complexity: O(n + m) Ω(1) ϴ(n + m), where n = number of entity_objects, m = number of entities.
	void update(unsigned int tps);
	
	Vector3F spawn_point, spawn_orientation;
private:
	static Object retrieve_object_data(const std::string& object_name, std::string resources_path, MDLF& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps);
	static EntityObject retrieve_entity_object_data(const std::string& entity_object_name, std::string resources_path, MDLF& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps);
	
	std::string filename;
	std::string resources_path;
	Vector3F gravity;
	std::vector<Object> objects;
	std::vector<Entity> entities;
	std::vector<EntityObject> entity_objects;
	std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light> base_lights;
};

#include "world.inl"

#endif