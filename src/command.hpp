#ifndef COMMAND_HPP
#define COMMAND_HPP
#include "world.hpp"
#include "audio.hpp"

class CommandCache
{
public:
	static void updateAlias(std::vector<std::string> aliasArgs);
	static std::vector<std::string> getAlias();
	static std::vector<std::string> aliasArgs;
	
	static std::vector<std::unique_ptr<AudioClip>> clips;
	static void addAudioClip(std::unique_ptr<AudioClip>&& clip);
	static void updateClip(AudioSource* source, Player& player);
	static void destroyChannelClips(int channel);
private:
	CommandCache();
	CommandCache(const CommandCache& copy) = delete;
	CommandCache(CommandCache&& move) = delete;
	CommandCache& operator=(const CommandCache& rhs) = delete;
	~CommandCache() = delete;
};

namespace Commands
{
	void inputCommand(std::string cmd, std::string resources_path, World& world, Player& player, const Shader& shader);
	void loadWorld(std::vector<std::string> args, std::string resources_path, World& world);
	void exportWorld(std::vector<std::string> args, World& world);
	void addObject(std::vector<std::string> args, World& world, Player& player, bool printResults);
	void addEntityObject(std::vector<std::string> args, World& world, Player& player, bool printResults);
	void setAlias(std::vector<std::string> args);
	void reloadWorld(std::vector<std::string> args, std::string resources_path, World& world, bool printResults);
	void updateWorld(World& world, std::string resources_path, bool printResults);
	void setSpeed(float speed);
	void printSpeed();
	void teleport(std::vector<std::string> args, Player& player);
	void roundLocation(Player& player);
	void setGravity(std::vector<std::string> args, World& world, bool printResults);
	void setSpawnPoint(std::vector<std::string> args, World& world, bool printResults);
	void setSpawnOrientation(std::vector<std::string> args, World& world, bool printResults);
	void addLight(std::vector<std::string> args, World& world, Player& player, const Shader& shader, bool printResults);
	void toggleMusic();
	void setVolume(std::vector<std::string> args);
	void printVolume();
	void playAudio(std::vector<std::string> args, bool printResults, Player& player);
	void scheduleAsyncDelayedMessage(std::vector<std::string> args, bool printResults);
	void scheduleAsyncDelayedCmd(std::vector<std::string> args, std::string resources_path, World& world, Player& player, const Shader& shader, bool printResults);
}

#endif