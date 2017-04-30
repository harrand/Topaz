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
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
	for(unsigned int i = 0; i < CommandCache::clips.size(); i++/*std::unique_ptr<AudioClip>& clip : CommandCache::clips*/)
	{
		unsigned int prevSize = CommandCache::clips.size();
		auto lambda = [channel](const std::unique_ptr<AudioClip>& clip) -> bool{return clip->getChannel() == channel;};
		auto rem = std::remove_if(CommandCache::clips.begin(), CommandCache::clips.end(), lambda);
		CommandCache::clips.erase(rem, CommandCache::clips.end());
		if(CommandCache::clips.size() != prevSize)
			LogUtility::message("Clip belonging to the channel " + CastUtility::toString<int>(channel) + " has been destroyed.");
	}
}

void Commands::inputCommand(std::string cmd, World& world, Player& player, Shader& shader)
{
	std::vector<std::string> args;
	if(StringUtility::contains(cmd, ' '))
		args = StringUtility::splitString(cmd, ' ');
	else
		args.push_back(cmd);
	std::string cmdName = StringUtility::toLower(args.at(0));
	
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
	else
		LogUtility::warning("Unknown command. Maybe you made a typo?");
}

void Commands::loadWorld(std::vector<std::string> args, World& world)
{
	if(args.size() != 2)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	std::vector<Entity> entities = world.getEntities();
	std::string worldname = args.at(1);
	std::string link = (RES_POINT + "/worlds/" + worldname);
	world = World(link);
	for(unsigned int i = 0; i < entities.size(); i++)
		world.addEntity(entities.at(i));
	LogUtility::message("Now rendering the world '" + worldname + "' which has " + CastUtility::toString<unsigned int>(world.getSize()) + " elements.");
}

void Commands::exportWorld(std::vector<std::string> args, World& world)
{
	if(args.size() != 2)
	{
	LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	std::string worldname = args.at(1);
	world.exportWorld(worldname);
}

void Commands::addObject(std::vector<std::string> args, World& world, Player& player, bool printResults)
{
	if(args.size() != 8)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 8.");
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
		LogUtility::warning("Nonfatal Command Error: Unknown Mesh Name '" + meshName + "'.");
		return;
	}
	if(textureLink == "0")
	{
		LogUtility::warning("Nonfatal Command Error: Unknown Texture Name '" + textureName + "'.");
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
		
	world.addObject(Object(meshLink, textureLink, normalMapLink, parallaxMapLink, pos, rot, scale));
	if(printResults)
		LogUtility::message("Added the following to this world:\nMesh name = " + meshName + ", link = " + meshLink + ".\nTexture name = " + textureName + ", link = " + textureLink + ".\nNormalmap name = " + normalMapName + ", link = " + normalMapLink + ".\nParallaxmap name = " + parallaxMapName + ", link = " + parallaxMapLink + ".\nPosition = " + StringUtility::format(StringUtility::devectoriseList3F(pos)) + ".\nRotation = " + StringUtility::format(StringUtility::devectoriseList3F(rot)) + ".\nScale = " + StringUtility::format(StringUtility::devectoriseList3F(scale)) + ".");
}

void Commands::addEntityObject(std::vector<std::string> args, World& world, Player& player, bool printResults)
{
	if(args.size() != 9)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 9.");
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
		LogUtility::warning("Nonfatal Command Error: Unknown Mesh Name '" + meshName + "'.");
		return;
	}
	if(textureLink == "0")
	{
		LogUtility::warning("Nonfatal Command Error: Unknown Texture Name '" + textureName + "'.");
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
	
	world.addEntityObject(EntityObject(meshLink, textureLink, normalMapLink, parallaxMapLink, mass, pos, rot, scale));
	if(printResults)
		LogUtility::message("Added the following to this world:\nMesh name = " + meshName + ", link = " + meshLink + ".\nTexture name = " + textureName + ", link = " + textureLink + ".\nNormalmap name = " + normalMapName + ", link = " + normalMapLink + ".\nParallaxmap name = " + parallaxMapName + ", link = " + parallaxMapLink + ".\nMass = " + CastUtility::toString<float>(mass) + ".\nPosition = " + StringUtility::format(StringUtility::devectoriseList3F(pos)) + ".\nRotation = " + StringUtility::format(StringUtility::devectoriseList3F(rot)) + ".\nScale = " + StringUtility::format(StringUtility::devectoriseList3F(scale)) + ".");
}

void Commands::setAlias(std::vector<std::string> args)
{
	args.erase(args.begin());
	CommandCache::updateAlias(args);
	std::string msg = "alias: ";
	for(auto& arg : args)
		msg += arg + " ";
	LogUtility::message(msg);
}

void Commands::reloadWorld(std::vector<std::string> args, World& world, bool printResults)
{
	args.resize(2); // Resize not reserve; resize will add empty elements in but reserve will not (so with reserve args.at(1) will still crash)
	args.at(1) = world.getFileName();
	std::string toErase = RES_POINT + "/worlds/";
	args.at(1).erase(args.at(1).find(toErase), toErase.length());
	Commands::loadWorld(args, world);
	if(printResults)
		LogUtility::message("Successfully reloaded the world. (world link " + world.getFileName() + ").");
}

void Commands::updateWorld(World& world, bool printResults)
{
	std::string worldLink = world.getFileName(), worldName = worldLink;
	std::string toRemove = RES_POINT + "/worlds/";
	worldName.erase(worldName.find(toRemove), toRemove.length());
	world.exportWorld(worldName);
	std::vector<std::string> args = std::vector<std::string>();
	args.push_back("loadworld");
	args.push_back(worldName);
	Commands::loadWorld(args, world);
	if(printResults)
		LogUtility::message("Successfully updated all new objects to the world named " + worldName + " (world link " + worldLink + ").");
}

void Commands::setSpeed(float speed)
{
	MDLF output(RawFile(RES_POINT + "/resources.data"));
	output.deleteTag("speed");
	output.addTag("speed", CastUtility::toString<float>(speed));
	LogUtility::message("Setting speed to " + CastUtility::toString<float>(speed) + ".");
}

void Commands::printSpeed()
{
	MDLF output(RawFile(RES_POINT + "/resources.data"));
	LogUtility::message("Current speed = " + output.getTag("speed") + ".");
}

void Commands::teleport(std::vector<std::string> args, Player& player)
{
	if(args.size() != 2)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	std::vector<std::string> teleSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F tele = Vector3F(CastUtility::fromString<float>(teleSplit.at(0)), CastUtility::fromString<float>(teleSplit.at(1)), CastUtility::fromString<float>(teleSplit.at(2)));
	player.getCamera().getPosR() = tele;
	LogUtility::message("Teleported.");
}

void Commands::roundLocation(Player& player)
{
	player.getCamera().getPosR() = Vector3F(round(player.getCamera().getPos().getX()), round(player.getCamera().getPos().getY()), round(player.getCamera().getPos().getZ()));
}

void Commands::setGravity(std::vector<std::string> args, World& world, bool printResults)
{
	if(args.size() != 2)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	Vector3F grav = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	world.setGravity(grav);
	if(printResults)
		LogUtility::message("Set gravity of the world '" + world.getFileName() + "' to " + StringUtility::format(StringUtility::devectoriseList3F(grav)) + " N.");
}

void Commands::setSpawnPoint(std::vector<std::string> args, World& world, bool printResults)
{
	if(args.size() != 2)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	Vector3F spawn = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	world.setSpawnPoint(spawn);
	if(printResults)
		LogUtility::message("Set spawnpoint of the world '" + world.getFileName() + "' to " + StringUtility::format(StringUtility::devectoriseList3F(spawn)) + ".");
}

void Commands::setSpawnOrientation(std::vector<std::string> args, World& world, bool printResults)
{
	if(args.size() != 2)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	Vector3F spawn = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	world.setSpawnOrientation(spawn);
	if(printResults)
		LogUtility::message("Set spawnorientation of the world '" + world.getFileName() + "' to " + StringUtility::format(StringUtility::devectoriseList3F(spawn)) + ".");
}

void Commands::addLight(std::vector<std::string> args, World& world, Player& player, Shader& shader, bool printResults)
{
	if(args.size() != 4)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 4.");
		return;
	}
	Vector3F pos, colour;
	if(args.at(1) == "me")
		pos = player.getPosition();
	else
		pos = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(1)));
	colour = StringUtility::vectoriseList3F(StringUtility::deformat(args.at(2)));
	float pow = CastUtility::fromString<float>(args.at(3));
	world.addLight(std::move(BaseLight(pos, colour, pow)), shader.getProgramHandle());
	if(printResults)
		LogUtility::message("Added a light at the position " + StringUtility::format(StringUtility::devectoriseList3F(pos)) + " with the RGB colour " + StringUtility::format(StringUtility::devectoriseList3F(colour)) + " and the power " + CastUtility::toString<float>(pow) + " W.");
}

void Commands::toggleMusic()
{
	if(Mix_PausedMusic())
	{
		Mix_ResumeMusic();
		LogUtility::message("Resumed music...");
	}
	else
	{
		Mix_PauseMusic();
		LogUtility::message("Paused music...");
	}
}

void Commands::setVolume(std::vector<std::string> args)
{
	if(args.size() != 2)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 2.");
		return;
	}
	Mix_VolumeMusic(CastUtility::fromString<float>(args.at(1)) * 128.0f/100.0f);
	LogUtility::message("Set volume of music to " + CastUtility::toString<float>(CastUtility::fromString<float>(args.at(1))) + "%");
}

void Commands::printVolume()
{
	LogUtility::message("Volume of music is " + CastUtility::toString<unsigned int>(Mix_VolumeMusic(-1) * 100.0f/128.0f) + "%");
}

void Commands::playAudio(std::vector<std::string> args, bool printResults, Player& player)
{
	if(args.size() != 3)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected 3.");
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
	CommandCache::addAudioClip(std::move(clip));
	Mix_ChannelFinished(CommandCache::destroyChannelClips);
	std::thread(CommandCache::updateClip, raw, std::ref(player)).detach();
	if(printResults)
		LogUtility::message("Playing the audio clip with the file-path " + filename + " at the position " + StringUtility::format(StringUtility::devectoriseList3F(pos)) +".");
}

void Commands::scheduleAsyncDelayedMessage(std::vector<std::string> args, bool printResults)
{
	if(args.size() < 3)
	{
		LogUtility::warning("Nonfatal Command Error: Unexpected quantity of args, got " + CastUtility::toString<unsigned int>(args.size()) + ", expected at least 3.");
		return;
	}
	unsigned int millisDelay = CastUtility::fromString<unsigned int>(args.at(1));
	if(printResults)
		LogUtility::message("Scheduling async delayed task of " + CastUtility::toString<unsigned int>(millisDelay) + "ms");
	std::string msg = "";
	for(unsigned int i = 2; i < args.size(); i++)
		msg += args.at(i) + (i == (args.size() - 1) ? "" : " ");
	Scheduler::asyncDelayedTask<void()>(millisDelay, [msg](){LogUtility::message(msg);});
}