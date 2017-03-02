#ifndef WORLD_HPP
#define WORLD_HPP
#include "object.hpp"
#include "entity.hpp"
#include "datatranslation.hpp"
#include "utility.hpp"
#include <functional>
#include <vector>

class World
{
public:
	World(std::string filename);
	const std::string getFileName() const;
	void addObject(Object obj);
	void addEntity(Entity& ent);
	void exportWorld(std::string worldName);
	void setGravity(Vector3F gravity = Vector3F());
	void setSpawnPoint(Vector3F spawnPoint = Vector3F());
	void setSpawnOrientation(Vector3F spawnOrientation = Vector3F());
	unsigned int getSize();
	std::vector<Object> getMembers();
	std::vector<std::reference_wrapper<Entity>> getEntities();
	Vector3F getGravity();
	Vector3F getSpawnPoint();
	Vector3F getSpawnOrientation();
	std::string getWorldLink();
private:
	Vector3F gravity, spawnPoint, spawnOrientation;
	const std::string filename;
	Object retrieveData(std::string objectName, MDLF& mdlf);
	std::vector<Object> members;
	std::vector<std::reference_wrapper<Entity>> entities;
};

#endif