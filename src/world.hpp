#ifndef WORLD_HPP
#define WORLD_HPP
#include "object.hpp"
#include "datatranslation.hpp"
#include "utility.hpp"
#include <vector>

class World
{
public:
	World(std::string filename);
	const std::string getFileName() const;
	void addObject(Object obj);
	void exportWorld(std::string worldName);
	void setGravity(Vector3F gravity = Vector3F());
	void setSpawnPoint(Vector3F spawnPoint = Vector3F());
	unsigned int getSize();
	std::vector<Object> getMembers();
	Vector3F getGravity();
	Vector3F getSpawnPoint();
	std::string getWorldLink();
private:
	Vector3F gravity, spawnPoint;
	const std::string filename;
	Object retrieveData(std::string objectName, MDLF& mdlf);
	std::vector<Object> members;
};

#endif