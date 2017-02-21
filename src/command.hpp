#ifndef COMMAND_HPP
#define COMMAND_HPP
#include "utility.hpp"
#include "world.hpp"
#include "object.hpp"

class CommandCache
{
public:
	static void updateDefaultObject(std::vector<std::string> addObjectArgs);
	static std::vector<std::string> getDefaultObject();
	static std::vector<std::string> defaultObjectArgs;
};

namespace Commands
{
	void inputCommand(std::string cmd, std::shared_ptr<World>& world, Camera& cam);
	void loadWorld(std::vector<std::string> args, std::shared_ptr<World>& world);
	void exportWorld(std::vector<std::string> args, std::shared_ptr<World>& world);
	void setDefaultObject(std::vector<std::string> args, std::shared_ptr<World>& world, Camera& cam, bool printResults);
	void addObject(std::vector<std::string> args, std::shared_ptr<World>& world, Camera& cam, bool printResults);
	void reloadWorld(std::shared_ptr<World>& world, bool printResults);
	void updateWorld(std::shared_ptr<World>& world, bool printResults);
	void setSpeed(float speed);
	void teleport(std::vector<std::string> args, Camera& cam);
	void roundLocation(Camera& cam);
}

#endif