#include "command.hpp"
#include "timekeeper.hpp"
#include <thread>

Command::Command(std::string name, std::string description, std::string usage): name(std::move(name)), description(std::move(description)), usage(std::move(usage)){}

const std::string& Command::getName() const
{
	return this->name;
}

const std::string& Command::getDescription() const
{
	return this->description;
}

const std::string& Command::getUsage() const
{
	return this->usage;
}

std::string& Command::getNameR()
{
	return this->name;
}

std::string& Command::getDescriptionR()
{
	return this->description;
}

std::string& Command::getUsageR()
{
	return this->usage;
}

std::size_t Command::getExpectedParameterSize() const
{
	return stringutility::splitString(this->usage, " ").size();
}

bool Command::operator==(const Command& rhs) const
{
	return this->name == rhs.getName() && this->description == rhs.getDescription() && this->usage == rhs.getUsage();
}

const std::unordered_set<Command*>& CommandExecutor::getCommands() const
{
	return this->commands;
}

std::unordered_set<Command*>& CommandExecutor::getCommandsR()
{
	return this->commands;
}

void CommandExecutor::registerCommand(Command* command)
{
	this->commands.insert(command);	
}

void CommandExecutor::deregisterCommand(Command* command)
{
	this->commands.erase(command);
}

void CommandExecutor::deregisterCommand(const std::string& command_name)
{
	for(auto& command : this->commands)
		if(command->getName() == command_name)
			this->deregisterCommand(command);
}

void CommandExecutor::operator()(const std::string& name, const std::vector<std::string>& args)
{
	for(auto command : this->commands)
		if(command->getName() == name)
			(*command)(args);
}

std::vector<std::string> CommandCache::aliasArgs = std::vector<std::string>();
std::vector<std::unique_ptr<AudioClip>> CommandCache::clips = std::vector<std::unique_ptr<AudioClip>>();

void CommandCache::updateAlias(std::vector<std::string> aliasArgs)
{
	CommandCache::aliasArgs = std::move(aliasArgs);
}

std::vector<std::string> CommandCache::getAlias()
{
	return CommandCache::aliasArgs;
}

void CommandCache::addAudioClip(std::unique_ptr<AudioClip>&& clip)
{
	CommandCache::clips.push_back(std::move(clip));
}

void CommandCache::updateClip(AudioSource* source, Player& player)
{
	bool exists = true;
	while(exists)
	{
		source->update(player);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::size_t size_cache = CommandCache::clips.size();
		bool found = false;
		if(CommandCache::clips.empty())
			break;
		for(auto& clip : CommandCache::clips)
		{
			if(CommandCache::clips.size() != size_cache)
			{
				// the other thread killed a clip and we need to fuck off immediately
				break;
			}
			if(source == clip.get())
			{
				found = true;
			}
		}
		exists = found;
	}
}

void CommandCache::destroyChannelClips(int channel)
{
	auto lambda = [channel](const std::unique_ptr<AudioClip>& clip) -> bool{return clip->getChannel() == channel;};
	auto rem = std::remove_if(CommandCache::clips.begin(), CommandCache::clips.end(), lambda);
	for(std::size_t i = 0; i < CommandCache::clips.size(); i++)
	{
		std::size_t prevSize = CommandCache::clips.size();
		CommandCache::clips.erase(rem, CommandCache::clips.end());
		if(CommandCache::clips.size() != prevSize)
			logutility::message("Clip belonging to the channel ", channel, " has been destroyed.");
	}
}

void Commands::inputCommand(std::string cmd, std::string resources_path, World& world, Player& player, const Shader& shader)
{
	std::vector<std::string> args;
	if(stringutility::contains(cmd, ' '))
		args = stringutility::splitString(cmd, ' ');
	else
		args.push_back(cmd);
	std::string cmdName = stringutility::toLower(args.at(0));
	
	if(cmdName == "loadworld")
		Commands::loadWorld(args, resources_path, world);
	else if(cmdName == "exportworld")
		Commands::exportWorld(args, world);
	else if(cmdName == "addobject")
		Commands::addObject(args, world, player, true);
	else if(cmdName == "addentityobject")
		Commands::addEntityObject(args, world, player, true);
	else if(cmdName == "alias")
		Commands::setAlias(args);
	else if(cmdName == "reloadworld")
		Commands::reloadWorld(args, resources_path, world, true);
	else if(cmdName == "updateworld")
		Commands::updateWorld(world, resources_path, true);
	else if(cmdName == "setspeed")
		Commands::setSpeed(castutility::fromString<float>(args.at(1)));
	else if(cmdName == "speed")
		Commands::printSpeed();
	else if(cmdName == "teleport")
		Commands::teleport(args, player);
	else if(cmdName == "roundlocation")
		Commands::roundLocation(player);
	else if(cmdName == "gravity")
		Commands::setGravity(args, world, true);
	else if(cmdName == "spawnpoint")
		Commands::setSpawnPoint(args, world, true);
	else if(cmdName == "spawnorientation")
		Commands::setSpawnOrientation(args, world, true);
	else if(cmdName == "addlight")
		Commands::addLight(args, world, player, shader, true);
	else if(cmdName == "pause")
		Commands::toggleMusic();
	else if(cmdName == "setvolume")
		Commands::setVolume(args);
	else if(cmdName == "volume")
		Commands::printVolume();
	else if(cmdName == "play")
		Commands::playAudio(args, true, player);
	else if(cmdName == "delayedmsg")
		Commands::scheduleAsyncDelayedMessage(args, true);
	else if(cmdName == "delayedcmd")
		Commands::scheduleAsyncDelayedCmd(args, resources_path, world, player, shader, true);
	else
		logutility::warning("Unknown command. Maybe you made a typo?");
}

void Commands::loadWorld(std::vector<std::string> args, std::string resources_path, World& world)
{
	if(args.size() != 2)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	std::vector<Entity> entities = world.getEntities();
	std::string worldname = args.at(1);
	std::string link = (worldname);
	world = World(link, resources_path);
	for(std::size_t i = 0; i < entities.size(); i++)
		world.addEntity(entities.at(i));
	logutility::message("Now rendering the world '", worldname, "' which has ", world.getSize(), " elements.");
}

void Commands::exportWorld(std::vector<std::string> args, World& world)
{
	if(args.size() != 2)
	{
	logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	std::string worldlink = args.at(1);
	world.exportWorld(worldlink);
}

void Commands::addObject(std::vector<std::string> args, World& world, Player& player, bool printResults)
{
	if(args.size() != 9)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 9.");
		return;
	}
	DataTranslation dt("resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string normalMapName = args.at(3);
	std::string parallaxMapName = args.at(4);
	std::string displacementMapName = args.at(5);
	std::string posStr = args.at(6);
	std::string rotStr = args.at(7);
	std::string scaleStr = args.at(8);

	Vector3F pos, rot, scale;
		
	std::string mesh_link = dt.getResourceLink(meshName);
	std::string texture_link = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	std::string parallaxMapLink = dt.getResourceLink(parallaxMapName);
	std::string displacementMapLink = dt.getResourceLink(displacementMapName);
	
	if(mesh_link == "0")
	{
		logutility::warning("Nonfatal Command Error: Unknown Mesh Name '",  meshName, "'.");
		return;
	}
	if(texture_link == "0")
	{
		logutility::warning("Nonfatal Command Error: Unknown Texture Name '", textureName, "'.");
		return;
	}
	
	if(posStr == "me")
		pos = player.getCamera().getPosition();
	else
		pos = stringutility::vectoriseList3<float>(stringutility::deformat(posStr));
		
	if(rotStr == "me")
		rot = Vector3F(player.getCamera().getRotation().getX(), player.getCamera().getRotation().getY(), player.getCamera().getRotation().getZ());
	else
		rot = stringutility::vectoriseList3<float>(stringutility::deformat(rotStr));
	
	scale = stringutility::vectoriseList3<float>(stringutility::deformat(scaleStr));
		
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	textures.push_back(std::make_pair(texture_link, Texture::TextureType::TEXTURE));
	textures.push_back(std::make_pair(normalMapLink, Texture::TextureType::NORMAL_MAP));
	textures.push_back(std::make_pair(parallaxMapLink, Texture::TextureType::PARALLAX_MAP));
	textures.push_back(std::make_pair(displacementMapLink, Texture::TextureType::DISPLACEMENT_MAP));
		
	world.addObject(Object(mesh_link, textures, pos, rot, scale));
	if(printResults)
		logutility::message("Added the following to this world:\nMesh name = ", meshName, ", link = ", mesh_link, ".\nTexture name = ", textureName, ", link = ", texture_link, ".\nNormalmap name = ", normalMapName, ", link = ", normalMapLink, ".\nParallaxmap name = ", parallaxMapName, ", link = ", parallaxMapLink, "\nDisplacementmap name = ", displacementMapName, ", link = ", displacementMapLink, ".\nPosition = ", stringutility::format(stringutility::devectoriseList3<float>(pos)), ".\nRotation = ", stringutility::format(stringutility::devectoriseList3<float>(rot)), ".\nScale = ", stringutility::format(stringutility::devectoriseList3<float>(scale)), ".");
}

void Commands::addEntityObject(std::vector<std::string> args, World& world, Player& player, bool printResults)
{
	if(args.size() != 10)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 10.");
		return;
	}
	DataTranslation dt("resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string normalMapName = args.at(3);
	std::string parallaxMapName = args.at(4);
	std::string displacementMapName = args.at(5);
	std::string massStr = args.at(6);
	std::string posStr = args.at(7);
	std::string rotStr = args.at(8);
	std::string scaleStr = args.at(9);

	Vector3F pos, rot, scale;
		
	std::string mesh_link = dt.getResourceLink(meshName);
	std::string texture_link = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	std::string parallaxMapLink = dt.getResourceLink(parallaxMapName);
	std::string displacementMapLink = dt.getResourceLink(displacementMapName);
	
	if(mesh_link == "0")
	{
		logutility::warning("Nonfatal Command Error: Unknown Mesh Name '", meshName, "'.");
		return;
	}
	if(texture_link == "0")
	{
		logutility::warning("Nonfatal Command Error: Unknown Texture Name '", textureName, "'.");
		return;
	}
	
	if(posStr == "me")
		pos = player.getCamera().getPosition();
	else
		pos = stringutility::vectoriseList3<float>(stringutility::deformat(posStr));
		
	if(rotStr == "me")
		rot = Vector3F(player.getCamera().getRotation().getX(), player.getCamera().getRotation().getY(), player.getCamera().getRotation().getZ());
	else
		rot = stringutility::vectoriseList3<float>(stringutility::deformat(rotStr));
	
	scale = stringutility::vectoriseList3<float>(stringutility::deformat(scaleStr));
	
	float mass = castutility::fromString<float>(massStr);
	
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	textures.push_back(std::make_pair(texture_link, Texture::TextureType::TEXTURE));
	textures.push_back(std::make_pair(normalMapLink, Texture::TextureType::NORMAL_MAP));
	textures.push_back(std::make_pair(parallaxMapLink, Texture::TextureType::PARALLAX_MAP));
	textures.push_back(std::make_pair(displacementMapLink, Texture::TextureType::DISPLACEMENT_MAP));
	
	world.addEntityObject(EntityObject(mesh_link, textures, mass, pos, rot, scale));
	if(printResults)
		logutility::message("Added the following to this world:\nMesh name = ", meshName, ", link = ", mesh_link, ".\nTexture name = ", textureName, ", link = ", texture_link, ".\nNormalmap name = ", normalMapName, ", link = ", normalMapLink, ".\nParallaxmap name = ", parallaxMapName, ", link = ", parallaxMapLink, "\nDisplacementmap name = ", displacementMapName, ", link = ", displacementMapLink, ".\nMass = ", mass, ".\nPosition = ", stringutility::format(stringutility::devectoriseList3<float>(pos)), ".\nRotation = ", stringutility::format(stringutility::devectoriseList3<float>(rot)), ".\nScale = ", stringutility::format(stringutility::devectoriseList3<float>(scale)), ".");
}

void Commands::setAlias(std::vector<std::string> args)
{
	args.erase(args.begin());
	CommandCache::updateAlias(args);
	std::string msg = "alias: ";
	for(auto& arg : args)
		msg += arg + " ";
	logutility::message(msg);
}

void Commands::reloadWorld(std::vector<std::string> args, std::string resources_path, World& world, bool printResults)
{
	args.resize(2); // Resize not reserve; resize will add empty elements in but reserve will not (so with reserve args.at(1) will still crash)
	args.at(1) = world.getFileName();
	Commands::loadWorld(args, resources_path, world);
	if(printResults)
		logutility::message("Successfully reloaded the world. (world link ", world.getFileName(), ").");
}

void Commands::updateWorld(World& world, std::string resources_path, bool printResults)
{
	std::string worldLink = world.getFileName();
	world.exportWorld(worldLink);
	std::vector<std::string> args = std::vector<std::string>();
	args.push_back("loadworld");
	args.push_back(worldLink);
	Commands::loadWorld(args, resources_path, world);
	if(printResults)
		logutility::message("Successfully updated all new objects to world link ", worldLink, ".");
}

void Commands::setSpeed(float speed)
{
	MDLF output(RawFile("resources.data"));
	output.deleteTag("speed");
	output.addTag("speed", castutility::toString<float>(speed));
	logutility::message("Setting speed to ", speed, ".");
}

void Commands::printSpeed()
{
	MDLF output(RawFile("resources.data"));
	logutility::message("Current speed = ", output.getTag("speed"), ".");
}

void Commands::teleport(std::vector<std::string> args, Player& player)
{
	if(args.size() != 2)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	std::vector<std::string> teleSplit = stringutility::splitString(stringutility::replaceAllChar(stringutility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F tele = Vector3F(castutility::fromString<float>(teleSplit.at(0)), castutility::fromString<float>(teleSplit.at(1)), castutility::fromString<float>(teleSplit.at(2)));
	player.getCamera().getPositionR() = tele;
	logutility::message("Teleported.");
}

void Commands::roundLocation(Player& player)
{
	player.getCamera().getPositionR() = Vector3F(round(player.getCamera().getPosition().getX()), round(player.getCamera().getPosition().getY()), round(player.getCamera().getPosition().getZ()));
}

void Commands::setGravity(std::vector<std::string> args, World& world, bool printResults)
{
	if(args.size() != 2)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	Vector3F grav = stringutility::vectoriseList3<float>(stringutility::deformat(args.at(1)));
	world.setGravity(grav);
	if(printResults)
		logutility::message("Set gravity of the world '", world.getFileName(), "' to ", stringutility::format(stringutility::devectoriseList3<float>(grav)), " N.");
}

void Commands::setSpawnPoint(std::vector<std::string> args, World& world, bool printResults)
{
	if(args.size() != 2)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	Vector3F spawn = stringutility::vectoriseList3<float>(stringutility::deformat(args.at(1)));
	world.setSpawnPoint(spawn);
	if(printResults)
		logutility::message("Set spawnpoint of the world '", world.getFileName(), "' to ", stringutility::format(stringutility::devectoriseList3<float>(spawn)), ".");
}

void Commands::setSpawnOrientation(std::vector<std::string> args, World& world, bool printResults)
{
	if(args.size() != 2)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	Vector3F spawn = stringutility::vectoriseList3<float>(stringutility::deformat(args.at(1)));
	world.setSpawnOrientation(spawn);
	if(printResults)
		logutility::message("Set spawnorientation of the world '", world.getFileName(), "' to ", stringutility::format(stringutility::devectoriseList3<float>(spawn)), ".");
}

void Commands::addLight(std::vector<std::string> args, World& world, Player& player, const Shader& shader, bool printResults)
{
	if(args.size() != 4)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 4.");
		return;
	}
	Vector3F pos, colour;
	if(args.at(1) == "me")
		pos = player.getPosition();
	else
		pos = stringutility::vectoriseList3<float>(stringutility::deformat(args.at(1)));
	colour = stringutility::vectoriseList3<float>(stringutility::deformat(args.at(2)));
	float pow = castutility::fromString<float>(args.at(3));
	world.addLight(std::move(BaseLight(pos, colour, pow)), shader.getProgramHandle());
	if(printResults)
		logutility::message("Added a light at the position ", stringutility::format(stringutility::devectoriseList3<float>(pos)), " with the RGB colour ", stringutility::format(stringutility::devectoriseList3<float>(colour)), " and the power ", pow," W.");
}

void Commands::toggleMusic()
{
	if(Mix_PausedMusic())
	{
		Mix_ResumeMusic();
		logutility::message("Resumed music...");
	}
	else
	{
		Mix_PauseMusic();
		logutility::message("Paused music...");
	}
}

void Commands::setVolume(std::vector<std::string> args)
{
	if(args.size() != 2)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected 2.");
		return;
	}
	Mix_VolumeMusic(castutility::fromString<float>(args.at(1)) * 128.0f/100.0f);
	logutility::message("Set volume of music to ", castutility::fromString<float>(args.at(1)), "%");
}

void Commands::printVolume()
{
	logutility::message("Volume of music is ", (Mix_VolumeMusic(-1) * 100.0f/128.0f), "%");
}

void Commands::playAudio(std::vector<std::string> args, bool printResults, Player& player)
{
	if(args.size() != 3)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(),", expected 3.");
		return;
	}
	std::string filename = args.at(1);
	Vector3F pos;
	if(args.at(2) == "me")
		pos = player.getPosition();
	else
		pos = stringutility::vectoriseList3<float>(stringutility::deformat(args.at(2)));
	std::unique_ptr<AudioSource> clip = std::make_unique<AudioSource>(filename, pos);
	clip->play();
	AudioSource* raw = clip.get();
	CommandCache::addAudioClip(std::move(clip));
	Mix_ChannelFinished(CommandCache::destroyChannelClips);
	std::thread(CommandCache::updateClip, raw, std::ref(player)).detach();
	if(printResults)
		logutility::message("Playing the audio clip with the file-path ", filename, " at the position ", stringutility::format(stringutility::devectoriseList3<float>(pos)), ".");
}

void Commands::scheduleAsyncDelayedMessage(std::vector<std::string> args, bool printResults)
{
	if(args.size() < 3)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected at least 3.");
		return;
	}
	unsigned int millisDelay = castutility::fromString<unsigned int>(args.at(1));
	if(printResults)
		logutility::message("Scheduling async delayed task of ", millisDelay, "ms");
	std::string msg = "";
	for(std::size_t i = 2; i < args.size(); i++)
		msg += args.at(i) + (i == (args.size() - 1) ? "" : " ");
	std::function<void(std::string)> printMsg([](std::string msg)->void{logutility::message(msg);});
	Scheduler::asyncDelayedTask<void, std::string>(millisDelay, printMsg, msg);
}

void Commands::scheduleAsyncDelayedCmd(std::vector<std::string> args, std::string resources_path, World& world, Player& player, const Shader& shader, bool printResults)
{
	if(args.size() < 3)
	{
		logutility::warning("Nonfatal Command Error: Unexpected quantity of args, got ", args.size(), ", expected at least 3.");
		return;
	}
	unsigned int millisDelay = castutility::fromString<unsigned int>(args.at(1));
	if(printResults)
		logutility::message("Scheduling async delayed task of ", millisDelay, "ms to execute a command.");
	std::string cmd = "";
	for(std::size_t i = 2; i < args.size(); i++)
		cmd += args.at(i) + (i == (args.size() - 1) ? "" : " ");
	std::function<void(std::string, std::string, std::reference_wrapper<World>, std::reference_wrapper<Player>, std::reference_wrapper<const Shader>)> inputCmd([](std::string cmd, std::string resources_path, std::reference_wrapper<World> world, std::reference_wrapper<Player> player, std::reference_wrapper<const Shader> shader)->void{Commands::inputCommand(cmd, resources_path, world, player, shader);});
	Scheduler::asyncDelayedTask<void, std::string, std::string, std::reference_wrapper<World>, std::reference_wrapper<Player>, std::reference_wrapper<const Shader>>(millisDelay, inputCmd, cmd, resources_path, std::ref(world), std::ref(player), std::cref(shader));
}