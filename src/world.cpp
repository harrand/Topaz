#include "world.hpp"

World::World(std::string filename): filename(filename)
{
	MDLF input(RawFile(this->filename));
	std::vector<std::string> objectList = input.getSequence("objects");
	for(unsigned int i = 0; i < objectList.size(); i++)
	{
		std::string objectName = objectList.at(i);
		Object obj = this->retrieveData(objectName, input);
		this->addObject(obj);
	}
	std::string spawnPointStr = input.getTag("spawnpoint"), spawnOrientationStr = input.getTag("spawnorientation"), gravStr = input.getTag("gravity");
	std::vector<std::string> spawnPointSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(spawnPointStr, '[', ""), ']', ""), ',');
	std::vector<std::string> spawnOrientationSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(spawnOrientationStr, '[', ""), ']', ""), ',');
	std::vector<std::string> gravSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(gravStr, '[', ""), ']', ""), ',');
	this->spawnPoint = Vector3F(CastUtility::fromString<float>(spawnPointSplit.at(0)), CastUtility::fromString<float>(spawnPointSplit.at(1)), CastUtility::fromString<float>(spawnPointSplit.at(2)));
	this->spawnOrientation = Vector3F(CastUtility::fromString<float>(spawnOrientationSplit.at(0)), CastUtility::fromString<float>(spawnOrientationSplit.at(1)), CastUtility::fromString<float>(spawnOrientationSplit.at(2)));
	this->gravity = Vector3F(CastUtility::fromString<float>(gravSplit.at(0)), CastUtility::fromString<float>(gravSplit.at(1)), CastUtility::fromString<float>(gravSplit.at(2)));
}

const std::string World::getFileName() const
{
	return this->filename;
}

void World::addObject(Object obj)
{
	this->members.push_back(obj);
}

//temp
#include <iostream>

void World::addEntity(Entity& ent)
{
	if(ent.getForces().find("gravity") != ent.getForces().end())
	{
		std::cout << "Entity already has a gravity force, so remove it before adding a new one.\n";
		ent.removeForce("gravity");
	}
	std::cout << "Added a new gravity force:" << this->getGravity().getX() << ", " << this->getGravity().getY() << ", " << this->getGravity().getZ() << "\n";
	ent.applyForce("gravity", Force(this->getGravity()));
	this->entities.push_back(ent);
}

void World::exportWorld(std::string worldName)
{
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string worldLink = RES_POINT + "/data/worlds/" + worldName;
	MDLF output = MDLF(RawFile(worldLink));
	output.getRawFile().clear();
	std::vector<std::string> objectList;
	output.deleteSequence("objects");
	std::string gravLink = "[" + StringUtility::toString(this->gravity.getX()) + ", " + StringUtility::toString(this->gravity.getY()) + ", " + StringUtility::toString(this->gravity.getZ()) + "]";
	std::string spawnPointLink = "[" + StringUtility::toString(this->spawnPoint.getX()) + ", " + StringUtility::toString(this->spawnPoint.getY()) + ", " + StringUtility::toString(this->spawnPoint.getZ()) + "]";
	std::string spawnOrientationLink = "[" + StringUtility::toString(this->spawnOrientation.getX()) + ", " + StringUtility::toString(this->spawnOrientation.getY()) + ", " + StringUtility::toString(this->spawnOrientation.getZ()) + "]";
	output.deleteTag("gravity");
	output.deleteTag("spawnpoint");
	output.deleteTag("spawnorientation");
	output.addTag("gravity", gravLink);
	output.addTag("spawnpoint", spawnPointLink);
	output.addTag("spawnorientation", spawnOrientationLink);
	for(unsigned int i = 0; i < this->members.size(); i++)
	{
		std::string objectName = "object" + StringUtility::toString(i);
		objectList.push_back(objectName);
		Object curObj = this->members.at(i);
		std::string meshLink = curObj.getMeshLink();
		std::string textureLink = curObj.getTextureLink();
		
		Vector3F pos = curObj.getPos(), rot = curObj.getRot(), scale = curObj.getScale();
		
		std::string posLink = "[" + StringUtility::toString(pos.getX()) + ", " + StringUtility::toString(pos.getY()) + ", " + StringUtility::toString(pos.getZ()) + "]";
		std::string rotLink = "[" + StringUtility::toString(rot.getX()) + ", " + StringUtility::toString(rot.getY()) + ", " + StringUtility::toString(rot.getZ()) + "]";
		std::string scaleLink = "[" + StringUtility::toString(scale.getX()) + ", " + StringUtility::toString(scale.getY()) + ", " + StringUtility::toString(scale.getZ()) + "]";
		
		std::string meshName = dt.getResourceName(meshLink);
		std::string textureName = dt.getResourceName(textureLink);
		
		output.deleteTag(objectName + ".mesh");
		output.deleteTag(objectName + ".texture");
		output.deleteTag(objectName + ".pos");
		output.deleteTag(objectName + ".rot");
		output.deleteTag(objectName + ".scale");
		
		output.addTag(objectName + ".mesh", meshName);
		output.addTag(objectName + ".texture", textureName);
		output.addTag(objectName + ".pos", posLink);
		output.addTag(objectName + ".rot", rotLink);
		output.addTag(objectName + ".scale", scaleLink);
	}
	output.addSequence("objects", objectList);
}

void World::setGravity(Vector3F gravity)
{
	this->gravity = gravity;
}

void World::setSpawnPoint(Vector3F spawnPoint)
{
	this->spawnPoint = spawnPoint;
}

void World::setSpawnOrientation(Vector3F spawnOrientation)
{
	this->spawnOrientation = spawnOrientation;
}

unsigned int World::getSize()
{
	return this->members.size();
}

std::vector<Object> World::getMembers()
{
	return this->members;
}

std::vector<std::reference_wrapper<Entity>> World::getEntities()
{
	return this->entities;
}

Vector3F World::getGravity()
{
	return this->gravity;
}

Vector3F World::getSpawnPoint()
{
	return this->spawnPoint;
}

Vector3F World::getSpawnOrientation()
{
	return this->spawnOrientation;
}

std::string World::getWorldLink()
{
	return this->filename;
}

Object World::retrieveData(std::string objectName, MDLF& mdlf)
{
	std::string meshName = mdlf.getTag(objectName + ".mesh");
	std::string textureName = mdlf.getTag(objectName + ".texture");
	std::string positionStr = mdlf.getTag(objectName + ".pos");
	std::string rotationStr = mdlf.getTag(objectName + ".rot");
	std::string scaleStr = mdlf.getTag(objectName + ".scale");
	
	DataTranslation dt(RES_POINT + "/resources.data");
	
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	
	std::vector<std::string> posData = StringUtility::splitString(StringUtility::replaceAllChar((StringUtility::replaceAllChar(positionStr, '[', "")), ']', ""), ',');
	float posX = CastUtility::fromString<float>(posData.at(0));
	float posY = CastUtility::fromString<float>(posData.at(1));
	float posZ = CastUtility::fromString<float>(posData.at(2));
	Vector3F pos(posX, posY, posZ);
	
	std::vector<std::string> rotData = StringUtility::splitString(StringUtility::replaceAllChar((StringUtility::replaceAllChar(rotationStr, '[', "")), ']', ""), ',');
	float rotX = CastUtility::fromString<float>(rotData.at(0));
	float rotY = CastUtility::fromString<float>(rotData.at(1));
	float rotZ = CastUtility::fromString<float>(rotData.at(2));
	Vector3F rot(rotX, rotY, rotZ);
	
	std::vector<std::string> scaleData = StringUtility::splitString(StringUtility::replaceAllChar((StringUtility::replaceAllChar(scaleStr, '[', "")), ']', ""), ',');
	float scaleX = CastUtility::fromString<float>(scaleData.at(0));
	float scaleY = CastUtility::fromString<float>(scaleData.at(1));
	float scaleZ = CastUtility::fromString<float>(scaleData.at(2));
	Vector3F scale(scaleX, scaleY, scaleZ);
	
	return Object(meshLink, textureLink, pos, rot, scale);
}