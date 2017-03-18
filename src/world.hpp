#ifndef WORLD_HPP
#define WORLD_HPP
#include "entityobject.hpp"
#include "datatranslation.hpp"

class World
{
public:
	World(std::string filename);
	const std::string getFileName() const;
	void addObject(Object obj);
	void addEntity(Entity* ent);
	void addEntityObject(std::shared_ptr<EntityObject> eo);
	void exportWorld(std::string worldName);
	void setGravity(Vector3F gravity = Vector3F());
	void setSpawnPoint(Vector3F spawnPoint = Vector3F());
	void setSpawnOrientation(Vector3F spawnOrientation = Vector3F());
	
	void update(unsigned int fps, Camera& cam, Shader& shader, unsigned int width, unsigned int height, std::vector<std::shared_ptr<Mesh>> allMeshes, std::vector<std::shared_ptr<Texture>> allTextures, std::vector<std::shared_ptr<NormalMap>> allNormalMaps);
	unsigned int getSize();
	std::vector<Object> getMembers();
	std::vector<Entity*> getEntities();
	std::vector<std::shared_ptr<EntityObject>> getEntityObjects();
	Vector3F getGravity();
	Vector3F getSpawnPoint();
	Vector3F getSpawnOrientation();
	std::string getWorldLink();
private:
	Vector3F gravity, spawnPoint, spawnOrientation;
	const std::string filename;
	Object retrieveData(std::string objectName, MDLF& mdlf);
	std::shared_ptr<EntityObject> retrieveEOData(std::string eoName, MDLF& mdlf);
	std::vector<Object> members;
	std::vector<Entity*> entities;
	std::vector<std::shared_ptr<EntityObject>> entityObjects;
};

#endif