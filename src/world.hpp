#ifndef WORLD_HPP
#define WORLD_HPP
#include <map>
#include "entityobject.hpp"
#include "datatranslation.hpp"
#include "light.hpp"

class World
{
public:
	World(std::string filename);
	World(const World& copy);
	World(World&& move);
	World& operator=(const World& rhs) = delete;
	~World();
	
	const std::string getFileName() const;
	void addObject(Object obj);
	void addEntity(Entity* ent);
	void addEntityObject(std::unique_ptr<EntityObject>&& eo);
	void addLight(BaseLight&& light, GLuint shader_programHandle);
	void setGravity(Vector3F gravity = Vector3F());
	void setSpawnPoint(Vector3F spawnPoint = Vector3F());
	void setSpawnOrientation(Vector3F spawnOrientation = Vector3F());
	
	void exportWorld(const std::string& worldName) const;
	void update(unsigned int fps, Camera& cam, Shader& shader, unsigned int width, unsigned int height, const std::vector<std::unique_ptr<Mesh>>& allMeshes, const std::vector<std::unique_ptr<Texture>>& allTextures, const std::vector<std::unique_ptr<NormalMap>>& allNormalMaps, const std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps) const;
		
	const unsigned int getSize() const;
	const std::vector<Object>& getMembers() const;
	const std::vector<Entity*>& getEntities() const;
	const std::vector<std::unique_ptr<EntityObject>>& getEntityObjects() const;
	const Vector3F& getGravity() const;
	const Vector3F& getSpawnPoint() const;
	const Vector3F& getSpawnOrientation() const;
	const std::string& getWorldLink() const;
	const std::map<std::vector<GLuint>, std::unique_ptr<BaseLight>>& getLights() const;
private:
	static const unsigned int MAXIMUM_LIGHTS;
	Vector3F gravity, spawnPoint, spawnOrientation;
	const std::string filename;
	static Object retrieveData(const std::string& objectName, MDLF& mdlf);
	static std::unique_ptr<EntityObject> retrieveEOData(const std::string& eoName, MDLF& mdlf);
	std::vector<Object> members;
	std::vector<Entity*> entities;
	std::vector<std::unique_ptr<EntityObject>> entityObjects;
	std::map<std::vector<GLuint>, std::unique_ptr<BaseLight>> baseLights;
};

#endif