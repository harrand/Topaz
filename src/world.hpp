#ifndef WORLD_HPP
#define WORLD_HPP
#include "object.hpp"
#include "datatranslation.hpp"
#include <vector>

class World
{
public:
	World(std::string filename);
	World(Object* objects, unsigned int numObjects);
	void addObject(Object obj);
	void exportWorld(std::string worldName);
	unsigned int getSize();
	std::vector<Object> getMembers();
	std::string getWorldLink();
private:
	std::string filename;
	Object retrieveData(std::string objectName, MDLF& mdlf);
	std::vector<Object> members;
};

#endif