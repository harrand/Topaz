#include "command.hpp"

std::vector<std::string> CommandCache::aliasArgs = std::vector<std::string>();

void CommandCache::updateAlias(std::vector<std::string> aliasArgs)
{
	CommandCache::aliasArgs = aliasArgs;
}

std::vector<std::string> CommandCache::getAlias()
{
	return CommandCache::aliasArgs;
}

void Commands::inputCommand(std::string cmd, std::shared_ptr<World>& world, Player& player)
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
	else
		std::cout << "Unknown command. Maybe you made a typo?\n";
}

void Commands::loadWorld(std::vector<std::string> args, std::shared_ptr<World>& world)
{
	if(args.size() != 2)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 2.\n";
		return;
	}
	std::vector<Entity*> entities = world->getEntities();
	std::string worldname = args.at(1);
	std::string link = (RES_POINT + "/data/worlds/" + worldname);
	world = std::shared_ptr<World>(new World(link));
	for(unsigned int i = 0; i < entities.size(); i++)
		world->addEntity(entities.at(i));
	std::cout << "Now rendering the world '" << worldname << "' which has " << world->getSize() << " objects.\n";
}

void Commands::exportWorld(std::vector<std::string> args, std::shared_ptr<World>& world)
{
	if(args.size() != 2)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 2.\n";
		return;
	}
	std::string worldname = args.at(1);
	world->exportWorld(worldname);
}

void Commands::addObject(std::vector<std::string> args, std::shared_ptr<World>& world, Player& player, bool printResults)
{
	if(args.size() != 7)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 6.\n";
		return;
	}
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string normalMapName = args.at(3);
	std::string posStr = args.at(4);
	std::string rotStr = args.at(5);
	std::string scaleStr = args.at(6);

	Vector3F pos, rot, scale;
		
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	
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
	{
		pos = player.getCamera().getPos();
	}
	else
	{
		std::vector<std::string> posSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(posStr, '[', ""), ']', ""), ',');
		pos = Vector3F(CastUtility::fromString<float>(posSplit.at(0)), CastUtility::fromString<float>(posSplit.at(1)), CastUtility::fromString<float>(posSplit.at(2)));
	}
		
	if(rotStr == "me")
	{
		rot = Vector3F(player.getCamera().getRot().getX(), player.getCamera().getRot().getY(), player.getCamera().getRot().getZ());
	}
	else
	{
		std::vector<std::string> rotSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(rotStr, '[', ""), ']', ""), ',');
		rot = Vector3F(CastUtility::fromString<float>(rotSplit.at(0)), CastUtility::fromString<float>(rotSplit.at(1)), CastUtility::fromString<float>(rotSplit.at(2)));	
	}
		
	std::vector<std::string> scaleSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(scaleStr, '[', ""), ']', ""), ',');
	scale = Vector3F(CastUtility::fromString<float>(scaleSplit.at(0)), CastUtility::fromString<float>(scaleSplit.at(1)), CastUtility::fromString<float>(scaleSplit.at(2)));
		
	world->addObject(Object(meshLink, textureLink, normalMapLink, pos, rot, scale));
	if(printResults)
	{
		std::cout << "Added the following to this world:\n";
		std::cout << "Mesh name = " << meshName << ", link = " << meshLink << ".\n";
		std::cout << "Texture name = " << textureName << ", link = " << textureLink << ".\n";
		std::cout << "Normalmap name = " << normalMapName << ", link = " << normalMapLink << ".\n";
		std::cout << "Pos = [" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << "].\n";
		std::cout << "Rot = [" << rot.getX() << ", " << rot.getY() << ", " << rot.getZ() << "].\n";
		std::cout << "Scale = [" << scale.getX() << ", " << scale.getY() << ", " << scale.getZ() << "].\n";
	} 
}

void Commands::addEntityObject(std::vector<std::string> args, std::shared_ptr<World>& world, Player& player, bool printResults)
{
	if(args.size() != 8)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 6.\n";
		return;
	}
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string normalMapName = args.at(3);
	std::string massStr = args.at(4);
	std::string posStr = args.at(5);
	std::string rotStr = args.at(6);
	std::string scaleStr = args.at(7);

	Vector3F pos, rot, scale;
		
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	
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
	{
		pos = player.getCamera().getPos();
	}
	else
	{
		std::vector<std::string> posSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(posStr, '[', ""), ']', ""), ',');
		pos = Vector3F(CastUtility::fromString<float>(posSplit.at(0)), CastUtility::fromString<float>(posSplit.at(1)), CastUtility::fromString<float>(posSplit.at(2)));
	}
		
	if(rotStr == "me")
	{
		rot = Vector3F(player.getCamera().getRot().getX(), player.getCamera().getRot().getY(), player.getCamera().getRot().getZ());
	}
	else
	{
		std::vector<std::string> rotSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(rotStr, '[', ""), ']', ""), ',');
		rot = Vector3F(CastUtility::fromString<float>(rotSplit.at(0)), CastUtility::fromString<float>(rotSplit.at(1)), CastUtility::fromString<float>(rotSplit.at(2)));	
	}
		
	std::vector<std::string> scaleSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(scaleStr, '[', ""), ']', ""), ',');
	scale = Vector3F(CastUtility::fromString<float>(scaleSplit.at(0)), CastUtility::fromString<float>(scaleSplit.at(1)), CastUtility::fromString<float>(scaleSplit.at(2)));
	
	float mass = CastUtility::fromString<float>(massStr);
	
	world->addEntityObject(std::shared_ptr<EntityObject>(new EntityObject(meshLink, textureLink, normalMapLink, mass, pos, rot, scale)));
	if(printResults)
	{
		std::cout << "Added the following to this world:\n";
		std::cout << "Mesh name = " << meshName << ", link = " << meshLink << ".\n";
		std::cout << "Texture name = " << textureName << ", link = " << textureLink << ".\n";
		std::cout << "Normalmap name = " << normalMapName << ", link = " << normalMapLink << ".\n";
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

void Commands::reloadWorld(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults)
{
	std::cout << "Reloading the world with link " << world->getFileName() << ".\n";
	args.resize(2); // Resize not reserve; resize will add empty elements in but reserve will not (so with reserve args.at(1) will still crash)
	args.at(1) = world->getFileName();
	std::string toErase = RES_POINT + "/data/worlds/";
	args.at(1).erase(args.at(1).find(toErase), toErase.length());
	std::cout << "World NAME should be " << args.at(1) << ". Loading this world...\n";
	Commands::loadWorld(args, world);
	if(printResults)
		std::cout << "Successfully reloaded the world. (world link " << world->getFileName() << ").\n";
}

void Commands::updateWorld(std::shared_ptr<World>& world, bool printResults)
{
	std::string worldLink = world->getFileName(), worldName = worldLink;
	std::string toRemove = RES_POINT + "/data/worlds/";
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
	output.addTag("speed", StringUtility::toString(speed));
	std::cout << "Setting speed to " << speed << ".\n";
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

void Commands::setGravity(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults)
{
	std::vector<std::string> gravSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F grav = Vector3F(CastUtility::fromString<float>(gravSplit.at(0)), CastUtility::fromString<float>(gravSplit.at(1)), CastUtility::fromString<float>(gravSplit.at(2)));
	world->setGravity(grav);
	
	if(printResults)
		std::cout << "Set gravity of the world '" << world->getFileName() << "' to [" << grav.getX() << "," << grav.getY() << "," << grav.getZ() << "] N\n";
}

void Commands::setSpawnPoint(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults)
{
	std::vector<std::string> spawnSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F spawn = Vector3F(CastUtility::fromString<float>(spawnSplit.at(0)), CastUtility::fromString<float>(spawnSplit.at(1)), CastUtility::fromString<float>(spawnSplit.at(2)));
	world->setSpawnPoint(spawn);
	if(printResults)
		std::cout << "Set spawnpoint of the world '" << world->getFileName() << "' to [" << spawn.getX() << "," << spawn.getY() << "," << spawn.getZ() << "]\n";
}

void Commands::setSpawnOrientation(std::vector<std::string> args, std::shared_ptr<World>& world, bool printResults)
{
	std::vector<std::string> spawnSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F spawn = Vector3F(CastUtility::fromString<float>(spawnSplit.at(0)), CastUtility::fromString<float>(spawnSplit.at(1)), CastUtility::fromString<float>(spawnSplit.at(2)));
	world->setSpawnOrientation(spawn);
	if(printResults)
		std::cout << "Set spawnorientation of the world '" << world->getFileName() << "' to [" << spawn.getX() << "," << spawn.getY() << "," << spawn.getZ() << "]\n";
}