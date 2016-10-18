#ifndef COMMAND_HPP
#define COMMAND_HPP
#include "utility.hpp"
#include "world.hpp"
#include "object.hpp"

class Commands
{
public:
	static void loadWorld(std::vector<std::string> args, World*& world);
	static void exportWorld(std::vector<std::string> args, World*& world);
	static void addObject(std::vector<std::string> args, World*& world, Camera& cam, bool printResults);
	static void reloadWorld(World*& world, bool printResults);
private:
};

#endif