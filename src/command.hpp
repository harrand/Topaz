#ifndef COMMAND_HPP
#define COMMAND_HPP
#include "world.hpp"
#include "player.hpp"
#include "audio.hpp"

class CommandCache
{
public:
	static void updateAlias(std::vector<std::string> aliasArgs);
	static std::vector<std::string> getAlias();
	static std::vector<std::string> aliasArgs;
	
	static std::vector<std::unique_ptr<AudioClip>> clips;
	static void addAudioClip(std::unique_ptr<AudioClip>&& clip);
	static void destroyChannelClips(int channel);
};

namespace Commands
{
	void inputCommand(std::string cmd, std::unique_ptr<World>& world, Player& player);
	void loadWorld(std::vector<std::string> args, std::unique_ptr<World>& world);
	void exportWorld(std::vector<std::string> args, std::unique_ptr<World>& world);
	void addObject(std::vector<std::string> args, std::unique_ptr<World>& world, Player& player, bool printResults);
	void addEntityObject(std::vector<std::string> args, std::unique_ptr<World>& world, Player& player, bool printResults);
	void setAlias(std::vector<std::string> args);
	void reloadWorld(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults);
	void updateWorld(std::unique_ptr<World>& world, bool printResults);
	void setSpeed(float speed);
	void printSpeed();
	void teleport(std::vector<std::string> args, Player& player);
	void roundLocation(Player& player);
	void setGravity(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults);
	void setSpawnPoint(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults);
	void setSpawnOrientation(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults);
	void toggleMusic();
	void setVolume(std::vector<std::string> args);
	void printVolume();
	void playAudio(std::vector<std::string> args, bool printResults);
}

#endif