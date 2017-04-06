#ifndef COMMAND_HPP
#define COMMAND_HPP
#include "world.hpp"
#include "player.hpp"

class CommandCache
{
public:
	static void updateAlias(std::vector<std::string> aliasArgs);
	static std::vector<std::string> getAlias();
	static std::vector<std::string> aliasArgs;
};

namespace Commands
{
	void inputCommand(std::string cmd, std::shared_ptr<World>& world, Player& player);
	void loadWorld(std::vector<std::string> args, std::shared_ptr<World>& world);
	void exportWorld(std::vector<std::string> args, std::shared_ptr<World>& world);
	void addObject(std::vector<std::string> args, std::shared_ptr<World>& world, Player& player, bool printResults);
	void addEntityObject(std::vector<std::string> args, std::shared_ptr<World>& world, Player& player, bool printResults);
	void setAlias(std::vector<std::string> args);
	void reloadWorld(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults);
	void updateWorld(std::shared_ptr<World>& world, bool printResults);
	void setSpeed(float speed);
	void printSpeed();
	void teleport(std::vector<std::string> args, Player& player);
	void roundLocation(Player& player);
	void setGravity(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults);
	void setSpawnPoint(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults);
	void setSpawnOrientation(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults);
}

#endif