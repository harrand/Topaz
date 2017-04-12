#include "command.hpp"

std::vector<std::string> CommandCache::aliasArgs = std::vector<std::string>();
std::vector<std::unique_ptr<AudioClip>> CommandCache::clips = std::vector<std::unique_ptr<AudioClip>>();

void CommandCache::updateAlias(std::vector<std::string> aliasArgs)
{
	CommandCache::aliasArgs = aliasArgs;
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
		unsigned int size_cache = CommandCache::clips.size();
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
	for(std::unique_ptr<AudioClip>& clip : CommandCache::clips)
	{
		unsigned int prevSize = CommandCache::clips.size();
		auto lambda = [channel](const std::unique_ptr<AudioClip>& clip) -> bool{return clip->getChannel() == channel;};
		auto rem = std::remove_if(CommandCache::clips.begin(), CommandCache::clips.end(), lambda);
		CommandCache::clips.erase(rem, CommandCache::clips.end());
		if(CommandCache::clips.size() != prevSize)
			std::cout << "Clip belonging to the channel " << channel << " has been destroyed.\n";
	}
}

void Commands::inputCommand(std::string cmd, std::unique_ptr<World>& world, Player& player, Shader& shader)
{
	std::vector<std::string> args;
	if(StringUtility::contains(cmd, ' '))
		args = StringUtility::splitString(cmd, ' ');
	else
		args.push_back(cmd);
	std::string cmdName = args.at(0);
	
	if(cmdName == "loadworld")
		Commands::loadWorld(args, world);
	else if(cmdName == "exportworld")
		Commands::exportWorld(args, world);
	else if(cmdName == "addobject")
		Commands::addObject(args, world, player, true);
	else if(cmdName == "addentityobject")
		Commands::addEntityObject(args, world, player, true);
	else if(cmdName == "alias")
		Commands::setAlias(args);
	else if(cmdName == "reloadworld")
		Commands::reloadWorld(args, world, true);
	else if(cmdName == "updateworld")
		Commands::updateWorld(world, true);
	else if(cmdName == "setspeed")
		Commands::setSpeed(CastUtility::fromString<float>(args.at(1)));
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
	else if(cmdName == "addLight")
		Commands::addLight(args, world, player, shader);
	else if(cmdName == "pause")
		Commands::toggleMusic();
	else if(cmdName == "setvolume")
		Commands::setVolume(args);
	else if(cmdName == "volume")
		Commands::printVolume();
	else if(cmdName == "play")
		Commands::playAudio(args, true, player);
	else
		std::cout << "Unknown command. Maybe you made a typo?\n";
}

void Commands::loadWorld(std::vector<std::string> args, std::unique_ptr<World>& world)
{
	if(args.size() != 2)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 2.\n";
		return;
	}
	std::vector<Entity*> entities = world->getEntities();
	std::string worldname = args.at(1);
	std::string link = (RES_POINT + "/worlds/" + worldname);
	world = std::make_unique<World>(link);
	for(unsigned int i = 0; i < entities.size(); i++)
		world->addEntity(entities.at(i));
	std::cout << "Now rendering the world '" << worldname << "' which has " << world->getSize() << " objects.\n";
}

void Commands::exportWorld(std::vector<std::string> args, std::unique_ptr<World>& world)
{
	if(args.size() != 2)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 2.\n";
		return;
	}
	std::string worldname = args.at(1);
	world->exportWorld(worldname);
}

void Commands::addObject(std::vector<std::string> args, std::unique_ptr<World>& world, Player& player, bool printResults)
{
	if(args.size() != 8)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 8.\n";
		return;
	}
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string normalMapName = args.at(3);
	std::string parallaxMapName = args.at(4);
	std::string posStr = args.at(5);
	std::string rotStr = args.at(6);
	std::string scaleStr = args.at(7);

	Vector3F pos, rot, scale;
		
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	std::string parallaxMapLink = dt.getResourceLink(parallaxMapName);
	
	if(meshLink == "0")
	{
		std::cout << "Nonfatal Command Error: Unknown Mesh Name '" << meshName << "'.\n";
		return;
	}
	if(textureLink == "0")
	{
		std::cout << "Nonfatal Command Error: Unknown Texture Name '" << textureName << "'.\n";
		return;
	}
	
	if(posStr == "me")
		pos = player.getCamera().getPos();
	else
		pos = StringUtility::vectoriseList3F(StringUtility::deformat(posStr));
		
	if(rotStr == "me")
		rot = Vector3F(player.getCamera().getRot().getX(), player.getCamera().getRot().getY(), player.getCamera().getRot().getZ());
	else
		rot = StringUtility::vectoriseList3F(StringUtility::deformat(rotStr));
	
	scale = StringUtility::vectoriseList3F(StringUtility::deformat(scaleStr));
		
	world->addObject(Object(meshLink, textureLink, normalMapLink, parallaxMapLink, pos, rot, scale));
	if(printResults)
	{
		std::cout << "Added the following to this world:\n";
		std::cout << "Mesh name = " << meshName << ", link = " << meshLink << ".\n";
		std::cout << "Texture name = " << textureName << ", link = " << textureLink << ".\n";
		std::cout << "Normalmap name = " << normalMapName << ", link = " << normalMapLink << ".\n";
		std::cout << "Parallaxmap name = " << parallaxMapName << ", link = " << parallaxMapLink << ".\n";
		std::cout << "Pos = [" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << "].\n";
		std::cout << "Rot = [" << rot.getX() << ", " << rot.getY() << ", " << rot.getZ() << "].\n";
		std::cout << "Scale = [" << scale.getX() << ", " << scale.getY() << ", " << scale.getZ() << "].\n";
	} 
}

void Commands::addEntityObject(std::vector<std::string> args, std::unique_ptr<World>& world, Player& player, bool printResults)
{
	if(args.size() != 9)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 6.\n";
		return;
	}
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string normalMapName = args.at(3);
	std::string parallaxMapName = args.at(4);
	std::string massStr = args.at(5);
	std::string posStr = args.at(6);
	std::string rotStr = args.at(7);
	std::string scaleStr = args.at(8);

	Vector3F pos, rot, scale;
		
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	std::string parallaxMapLink = dt.getResourceLink(parallaxMapName);
	
	if(meshLink == "0")
	{
		std::cout << "Nonfatal Command Error: Unknown Mesh Name '" << meshName << "'.\n";
		return;
	}
	if(textureLink == "0")
	{
		std::cout << "Nonfatal Command Error: Unknown Texture Name '" << textureName << "'.\n";
		return;
	}
	
	if(posStr == "me")
		pos = player.getCamera().getPos();
	else
		pos = StringUtility::vectoriseList3F(StringUtility::deformat(posStr));
		
	if(rotStr == "me")
		rot = Vector3F(player.getCamera().getRot().getX(), player.getCamera().getRot().getY(), player.getCamera().getRot().getZ());
	else
		rot = StringUtility::vectoriseList3F(StringUtility::deformat(rotStr));
	
	scale = StringUtility::vectoriseList3F(StringUtility::deformat(scaleStr));
	
	float mass = CastUtility::fromString<float>(massStr);
	
	world->addEntityObject(std::make_unique<EntityObject>(meshLink, textureLink, normalMapLink, parallaxMapLink, mass, pos, rot, scale));
	if(printResults)
	{
		std::cout << "Added the following to this world:\n";
		std::cout << "Mesh name = " << meshName << ", link = " << meshLink << ".\n";
		std::cout << "Texture name = " << textureName << ", link = " << textureLink << ".\n";
		std::cout << "Normalmap name = " << normalMapName << ", link = " << normalMapLink << ".\n";
		std::cout << "Parallaxmap name = " << parallaxMapName << ", link = " << parallaxMapLink << ".\n";
		std::cout << "Mass = " << mass << "\n";
		std::cout << "Pos = [" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << "].\n";
		std::cout << "Rot = [" << rot.getX() << ", " << rot.getY() << ", " << rot.getZ() << "].\n";
		std::cout << "Scale = [" << scale.getX() << ", " << scale.getY() << ", " << scale.getZ() << "].\n";
	} 
}

void Commands::setAlias(std::vector<std::string> args)
{
	args.erase(args.begin());
	CommandCache::updateAlias(args);
	std::cout << "alias: ";
	for(unsigned int i = 0; i < args.size(); i++)
		std::cout << args.at(i) << " ";
	std::cout << "\n";
}

void Commands::reloadWorld(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults)
{
	std::cout << "Reloading the world with link " << world->getFileName() << ".\n";
	args.resize(2); // Resize not reserve; resize will add empty elements in but reserve will not (so with reserve args.at(1) will still crash)
	args.at(1) = world->getFileName();
	std::string toErase = RES_POINT + "/worlds/";
	args.at(1).erase(args.at(1).find(toErase), toErase.length());
	std::cout << "World NAME should be " << args.at(1) << ". Loading this world...\n";
	Commands::loadWorld(args, world);
	if(printResults)
		std::cout << "Successfully reloaded the world. (world link " << world->getFileName() << ").\n";
}

void Commands::updateWorld(std::unique_ptr<World>& world, bool printResults)
{
	std::string worldLink = world->getFileName(), worldName = worldLink;
	std::string toRemove = RES_POINT + "/worlds/";
	worldName.erase(worldName.find(toRemove), toRemove.length());
	world->exportWorld(worldName);
	std::vector<std::string> args = std::vector<std::string>();
	args.push_back("loadworld");
	args.push_back(worldName);
	Commands::loadWorld(args, world);
	if(printResults)
		std::cout << "Successfully updated all new objects to the world named " << worldName << " (world link " << worldLink << ").\n";
}

void Commands::setSpeed(float speed)
{
	MDLF output(RawFile(RES_POINT + "/resources.data"));
	output.deleteTag("speed");
	output.addTag("speed", CastUtility::toString(speed));
	std::cout << "Setting speed to " << speed << ".\n";
}

void Commands::printSpeed()
{
	MDLF output(RawFile(RES_POINT + "/resources.data"));
	std::cout << "Current speed = " << output.getTag("speed") << "\n";
}

void Commands::teleport(std::vector<std::string> args, Player& player)
{
	if(args.size() != 2)
	{
		std::cout << "Command failed: Expected 2 arguments, got " << args.size() << ".\n";
		return;
	}
	std::vector<std::string> teleSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F tele = Vector3F(CastUtility::fromString<float>(teleSplit.at(0)), CastUtility::fromString<float>(teleSplit.at(1)), CastUtility::fromString<float>(teleSplit.at(2)));
	player.getCamera().getPosR() = tele;
	std::cout << "Teleported.\n";
}

void Commands::roundLocation(Player& player)
{
	player.getCamera().getPosR() = Vector3F(round(player.getCamera().getPos().getX()), round(player.getCamera().getPos().getY()), round(player.getCamera().getPos().getZ()));
}

void Commands::setGravity(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults)
{
	if(args.size() != 2)
	{
		std::cout << "Command failed: Expected 2 arguments, got " << args.size() << ".\n";
		return;
	}
	Vector3F grav = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	world->setGravity(grav);
	if(printResults)
		std::cout << "Set gravity of the world '" << world->getFileName() << "' to [" << grav.getX() << "," << grav.getY() << "," << grav.getZ() << "] N\n";
}

void Commands::setSpawnPoint(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults)
{
	if(args.size() != 2)
	{
		std::cout << "Command failed: Expected 2 arguments, got " << args.size() << ".\n";
		return;
	}
	Vector3F spawn = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	world->setSpawnPoint(spawn);
	if(printResults)
		std::cout << "Set spawnpoint of the world '" << world->getFileName() << "' to [" << spawn.getX() << "," << spawn.getY() << "," << spawn.getZ() << "]\n";
}

void Commands::setSpawnOrientation(std::vector<std::string> args, std::unique_ptr<World>& world, bool printResults)
{
	if(args.size() != 2)
	{
		std::cout << "Command failed: Expected 2 arguments, got " << args.size() << ".\n";
		return;
	}
	Vector3F spawn = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	world->setSpawnOrientation(spawn);
	if(printResults)
		std::cout << "Set spawnorientation of the world '" << world->getFileName() << "' to [" << spawn.getX() << "," << spawn.getY() << "," << spawn.getZ() << "]\n";
}

void Commands::addLight(std::vector<std::string> args, std::unique_ptr<World>& world, Player& player, Shader& shader)
{
	if(args.size() != 4)
	{
		std::cout << "Command failed: Expected 4 arguments, got " << args.size() << ".\n";
		return;
	}
	Vector3F pos, colour;
	if(args.at(1) == "me")
		pos = player.getPosition();
	else
		pos = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	colour = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(2)));
	float pow = CastUtility::fromString<float>(args.at(3));
	world->addLight(std::move(BaseLight(pos, colour, pow)), shader.getProgramHandle());
	std::cout << "Added a light at the position [" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << "] with the colour [" << colour.getX() << ", " << colour.getY() << ", " << colour.getZ() << "] and the power " << pow << " watts.\n";
}

void Commands::toggleMusic()
{
	if(Mix_PausedMusic())
	{
		Mix_ResumeMusic();
		std::cout << "Resumed music...\n";
	}
	else
	{
		Mix_PauseMusic();
		std::cout << "Paused music...\n";
	}
}

void Commands::setVolume(std::vector<std::string> args)
{
	if(args.size() != 2)
	{
		std::cout << "Command failed: Expected 2 arguments, got " << args.size() << ".\n";
		return;
	}
	Mix_VolumeMusic(CastUtility::fromString<float>(args.at(1)) * 128.0f/100.0f);
	std::cout << "Set volume of music to " << CastUtility::fromString<float>(args.at(1)) << "%\n";
}

void Commands::printVolume()
{
	std::cout << "Volume of music is " << Mix_VolumeMusic(-1) * 100.0f/128.0f << "%\n";
}

void Commands::playAudio(std::vector<std::string> args, bool printResults, Player& player)
{
	if(args.size() != 3)
	{
		std::cout << "Command failed: Expected 3 arguments, got " << args.size() << ".\n";
		return;
	}
	std::string filename = RES_POINT + "/music/" + args.at(1);
	Vector3F pos;
	if(args.at(2) == "me")
		pos = player.getPosition();
	else
		pos = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(2)));
	std::unique_ptr<AudioSource> clip = std::make_unique<AudioSource>(filename, pos);
	clip->play();
	AudioSource* raw = clip.get();
	bool shouldStartThread = CommandCache::clips.empty();
	CommandCache::addAudioClip(std::move(clip));
	Mix_ChannelFinished(CommandCache::destroyChannelClips);
	if(shouldStartThread)
	{
		std::thread(CommandCache::updateClip, raw, std::ref(player)).detach();
		std::cout << "Started thread to manage audiosources.\n";
	}
	std::cout << "Playing the audio clip with the file-path " << filename << " at the position [" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << "]\n";
}