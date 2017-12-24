#ifndef WORLD_HPP
#define WORLD_HPP
#include <map>
#include <cstddef>
#include "entity.hpp"
#include "light.hpp"
#include "MDL/mdl.hpp"

namespace tz::scene
{
	constexpr char spawnpoint_tag_name[] = "spawnpoint";
	constexpr char spawnorientation_tag_name[] = "spawnorientation";
	constexpr char objects_sequence_name[] = "objects";
	constexpr char entityobjects_sequence_name[] = "entityobjects";
}

/*
	Contains Objects, EntityObjects and pretty much any Topaz renderable you can think of. Handles all their physics inputs aswell. Takes a filename in its constructor for the MDL data file containing scene data. Use this to store everything you want to draw.
*/
class Scene
{
public:
	// Engine cannot initialise Scene in its initialiser-list, so a default scene must be available. It needn't serve any functionality, however. Unfortunately one of the unavoidable drawbacks of C++.
	Scene();
	// Load a scene from an existing MDL file. Takes in all asset vectors. Should probably be replaced with just a const asset manager reference of some kind to read the data without all this verbosity.
	Scene(std::string filename, std::string resources_path, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps);
	Scene(const Scene& copy) = default;
	Scene(Scene&& move) = default;
	~Scene() = default;
	Scene& operator=(const Scene& rhs) = default;

	bool has_file_name() const;
	const std::string& get_file_name() const;
	// Complexity: O(1) amortised Ω(1) ϴ(1) amortised
	void add_object(Object3D obj);
	// Complexity: O(1) amortised Ω(1) ϴ(1) amortised
	void add_entity(Entity ent);
	// // Complexity: O(1) amortised Ω(1) ϴ(1) amortised
	void add_entity_object(EntityObject3D eo);
	// Complexity: O(n log n) Ω(log n) ϴ(log n), where n = number of existing lights.
	void add_light(Light light, GLuint shader_program_handle);
	template<class Element, typename... Args>
	Element& emplace(Args&&... args);
	template<typename... Args>
	Object3D& emplace_object(Args&&... args);
	template<typename... Args>
	Entity& emplace_entity(Args&&... args);
	template<typename... Args>
	EntityObject3D& emplace_entity_object(Args&&... args);
	void remove_object(const Object3D& obj);
	void remove_entity(const Entity& ent);
	void remove_entity_object(const EntityObject3D& eo);
	void remove_light(const Light& light);
	const std::vector<Object3D>& get_objects() const;
	const std::vector<Entity>& get_entities() const;
	const std::vector<EntityObject3D>& get_entity_objects() const;
	// Returns total number of Objects, Entities and EntityObjects in the scene.
	std::size_t get_size() const;
	const std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light>& get_lights() const;
	// Updates uniforms to currently bound shader, zeroing light values for all lights. Complexity: O(n) Ω(1) ϴ(n), where n = number of lights.
	void kill_lights();
	// Export scene data to a MDL file called scene_link. Complexity: O(n + m) Ω(1) ϴ(n + m), where n = number of objects and m = number of entity_objects.
	void export_scene(const std::string& scene_link) const;
	// Export scene data to a MDL file with the same name as the file which loaded this scene, overwriting it. Complexity: See Scene::export_scene.
	void save() const;
	// Render all elements in the scene from the perspective of the camera, attaching a shader and updating uniforms in the process. Width and height parameters required to generate projection matrices & correct aspect-ratio. This method should be invoked as often as possible, to smooth gameplay. Complexity: O(n + p + o) Ω(1) ϴ(n + p + o), where n = number of objects, p = number of entity_objects, o = number of lights.
	void render(const Camera& cam, Shader* shader, unsigned int width, unsigned int height);
	// Update all elements in the scene that obey some form of law of physics. Pass tps as the expected ticks-per-second, not the instantaneous tick per second. This function should be called per 'tick'. Complexity: O(n + m) Ω(1) ϴ(n + m), where n = number of entity_objects, m = number of entities.
	void update(unsigned int tps);
	
	Vector3F spawn_point, spawn_orientation;
private:
	static Object3D retrieve_object_data(const std::string& object_name, std::string resources_path, MDLF& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps);
	static EntityObject3D retrieve_entity_object_data(const std::string& entity_object_name, std::string resources_path, MDLF& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps);
	
	std::optional<std::string> filename;
	std::optional<std::string> resources_path;
	std::vector<Object3D> objects;
	std::vector<Entity> entities;
	std::vector<EntityObject3D> entity_objects;
	std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light> base_lights;
};

#include "scene.inl"

#endif