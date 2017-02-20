#include "command.hpp"

void Commands::inputCommand(std::string cmd, std::shared_ptr<World>& world, Camera& cam)
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
		Commands::addObject(args, world, cam, true);
	else if(cmdName == "reloadworld")
		Commands::reloadWorld(world, true);
	else if(cmdName == "setspeed")
		Commands::setSpeed(CastUtility::fromString<float>(args.at(1)));
	else if(cmdName == "teleport")
		Commands::teleport(args, cam);
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
	std::string worldname = args.at(1);
	std::string link = (RES_POINT + "/data/worlds/" + worldname);
	world = std::shared_ptr<World>(new World(link));
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

void Commands::addObject(std::vector<std::string> args, std::shared_ptr<World>& world, Camera& cam, bool printResults)
{
	if(args.size() != 6)
	{
		std::cout << "Nonfatal Command Error: Unexpected quantity of args, got " << args.size() << ", expected 6.\n";
		return;
	}
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string meshName = args.at(1);
	std::string textureName = args.at(2);
	std::string posStr = args.at(3);
	std::string rotStr = args.at(4);
	std::string scaleStr = args.at(5);

	Vector3F pos, rot, scale;
		
	std::string meshLink = dt.getMeshLink(meshName);
	std::string textureLink = dt.getTextureLink(textureName);
	
	if(meshLink == "_")
	{
		std::cout << "Nonfatal Command Error: Unknown Mesh Name '" << meshName << "'.\n";
		return;
	}
	if(textureLink == "_")
	{
		std::cout << "Nonfatal Command Error: Unknown Texture Name '" << textureName << "'.\n";
		return;
	}
	
	if(posStr == "me")
	{
		pos = cam.getPos();
	}
	else
	{
		std::vector<std::string> posSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(posStr, '[', ""), ']', ""), ',');
		pos = Vector3F(CastUtility::fromString<float>(posSplit.at(0)), CastUtility::fromString<float>(posSplit.at(1)), CastUtility::fromString<float>(posSplit.at(2)));
	}
		
	if(rotStr == "me")
	{
		rot = Vector3F(cam.getRot().getX(), cam.getRot().getY(), cam.getRot().getZ());
	}
	else
	{
		std::vector<std::string> rotSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(rotStr, '[', ""), ']', ""), ',');
		rot = Vector3F(CastUtility::fromString<float>(rotSplit.at(0)), CastUtility::fromString<float>(rotSplit.at(1)), CastUtility::fromString<float>(rotSplit.at(2)));	
	}
		
	std::vector<std::string> scaleSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(scaleStr, '[', ""), ']', ""), ',');
	scale = Vector3F(CastUtility::fromString<float>(scaleSplit.at(0)), CastUtility::fromString<float>(scaleSplit.at(1)), CastUtility::fromString<float>(scaleSplit.at(2)));
		
	world->addObject(Object(meshLink, textureLink, pos, rot, scale));
	if(printResults)
	{
		std::cout << "Added the following to this world:\n";
		std::cout << "Mesh name = " << meshName << ", link = " << meshLink << ".\n";
		std::cout << "Texture name = " << textureName << ", link = " << textureLink << ".\n";
		std::cout << "Pos = [" << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << "].\n";
		std::cout << "Rot = [" << rot.getX() << ", " << rot.getY() << ", " << rot.getZ() << "].\n";
		std::cout << "Scale = [" << scale.getX() << ", " << scale.getY() << ", " << scale.getZ() << "].\n";
	}
}

void Commands::reloadWorld(std::shared_ptr<World>& world, bool printResults)
{
	std::string worldLink = world->getFileName(), worldName = worldLink;
	//RES_POINT + "/data/worlds/" + worldName;
	std::string toRemove = RES_POINT + "/data/worlds/";
	worldName.erase(worldName.find(toRemove), toRemove.length());
	world->exportWorld(worldName);
	world = std::shared_ptr<World>(new World(worldLink));
	if(printResults)
		std::cout << "Successfully reloaded the world named " << worldName << " (world link " << worldLink << ").\n";
}

void Commands::setSpeed(float speed)
{
	MDLF output(RawFile(RES_POINT + "/resources.data"));
	output.deleteTag("speed");
	output.addTag("speed", StringUtility::toString(speed));
	std::cout << "Setting speed to " << speed << ".\n";
}

void Commands::teleport(std::vector<std::string> args, Camera& cam)
{
	if(args.size() != 2)
	{
		std::cout << "Command failed: Expected 2 arguments, got " << args.size() << ".\n";
		return;
	}
	std::vector<std::string> teleSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(args.at(1), '[', ""), ']', ""), ',');
	Vector3F tele = Vector3F(CastUtility::fromString<float>(teleSplit.at(0)), CastUtility::fromString<float>(teleSplit.at(1)), CastUtility::fromString<float>(teleSplit.at(2)));
	cam.getPosR() = tele;
	std::cout << "Teleported.\n";
}