#ifndef WORLD_HPP
#define WORLD_HPP
#include <map>
#include <cstddef>
#include "entityobject.hpp"
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
	
	const std::string& getFileName() const;
	void addObject(Object obj);
	void addEntity(Entity ent);
	void addEntityObject(EntityObject eo);
	void addLight(BaseLight light, GLuint shader_program_handle);
	void setGravity(Vector3F gravity = Vector3F());
	void setSpawnPoint(Vector3F spawn_point = Vector3F());
	void setSpawnOrientation(Vector3F spawn_orientation = Vector3F());
	void killLights();
	void exportWorld(const std::string& world_link) const;
	void render(Camera& cam, const Shader& shader, unsigned int width, unsigned int height, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps);
	void update(unsigned int tps);
	std::size_t getSize() const;
	const std::vector<Object>& getObjects() const;
	const std::vector<Entity>& getEntities() const;
	const std::vector<EntityObject>& getEntityObjects() const;
	std::vector<Object>& getObjectsR();
	std::vector<Entity>& getEntitiesR();
	std::vector<EntityObject>& getEntityObjectsR();
	const Vector3F& getGravity() const;
	const Vector3F& getSpawnPoint() const;
	const Vector3F& getSpawnOrientation() const;
	Vector3F& getGravityR();
	Vector3F& getSpawnPointR();
	Vector3F& getSpawnOrientationR();
	const std::string& getWorldLink() const;
	const std::map<std::vector<GLuint>, BaseLight>& getLights() const;
	std::map<std::vector<GLuint>, BaseLight>& getLightsR();
private:
	static Object retrieveObjectData(const std::string& object_name, std::string resources_path, MDLF& mdlf);
	static EntityObject retrieveEntityObjectData(const std::string& entity_object_name, std::string resources_path, MDLF& mdlf);
	static constexpr unsigned int MAXIMUM_LIGHTS = 8;
	std::string filename;
	std::string resources_path;
	Vector3F gravity, spawn_point, spawn_orientation;
	std::vector<Object> objects;
	std::vector<Entity> entities;
	std::vector<EntityObject> entity_objects;
	std::map<std::vector<GLuint>, BaseLight> base_lights;
};
#endif