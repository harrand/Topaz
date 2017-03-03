#include "world.hpp"

//temp
#include <iostream>

World::World(std::string filename): filename(filename)
{
	MDLF input(RawFile(this->filename));
	std::vector<std::string> objectList = input.getSequence("objects");
	std::vector<std::string> entityObjectList = input.getSequence("entityobjects");
	std::cout << "retrieved entityobject list of size " << entityObjectList.size() << ".\n";
	for(unsigned int i = 0; i < objectList.size(); i++)
	{
		std::string objectName = objectList.at(i);
		Object obj = this->retrieveData(objectName, input);
		this->addObject(obj);
	}
	for(unsigned int i = 0; i < entityObjectList.size(); i++)
	{
		std::string eoName = entityObjectList.at(i);
		std::cout << "obtaining " << eoName << "...\n";
		std::shared_ptr<EntityObject> eo = this->retrieveEOData(eoName, input);
		std::cout << "obtained " << eoName << "...\n";
		this->addEntityObject(eo);
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

void World::addEntity(Entity* ent)
{
	if(ent->getForces().find("gravity") != ent->getForces().end())
	{
		ent->removeForce("gravity");
	}
	ent->applyForce("gravity", Force(this->getGravity()));
	this->entities.push_back(ent);
}

void World::addEntityObject(std::shared_ptr<EntityObject> eo)
{
	if(eo->getForces().find("gravity") != eo->getForces().end())
	{
		eo->removeForce("gravity");
	}
	eo->applyForce("gravity", Force(this->getGravity()));
	this->entityObjects.push_back(eo);
}

void World::exportWorld(std::string worldName)
{
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string worldLink = RES_POINT + "/data/worlds/" + worldName;
	MDLF output = MDLF(RawFile(worldLink));
	output.getRawFile().clear();
	std::vector<std::string> objectList;
	std::vector<std::string> eoList;
	output.deleteSequence("objects");
	output.deleteSequence("entityobjects");
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
	for(unsigned int i = 0; i < this->entityObjects.size(); i++)
	{
		std::string eoName = "eo" + StringUtility::toString(i);
		eoList.push_back(eoName);
		std::shared_ptr<EntityObject> curEO = this->entityObjects.at(i);
		std::string meshLink = curEO->getMeshLink();
		std::string textureLink = curEO->getTextureLink();
		
		std::string massStr = StringUtility::toString(curEO->getMass());
		
		Vector3F pos = curEO->getPos(), rot = curEO->getRot(), scale = curEO->getScale();
		
		std::string posLink = "[" + StringUtility::toString(pos.getX()) + ", " + StringUtility::toString(pos.getY()) + ", " + StringUtility::toString(pos.getZ()) + "]";
		std::string rotLink = "[" + StringUtility::toString(rot.getX()) + ", " + StringUtility::toString(rot.getY()) + ", " + StringUtility::toString(rot.getZ()) + "]";
		std::string scaleLink = "[" + StringUtility::toString(scale.getX()) + ", " + StringUtility::toString(scale.getY()) + ", " + StringUtility::toString(scale.getZ()) + "]";
		
		std::string meshName = dt.getResourceName(meshLink);
		std::string textureName = dt.getResourceName(textureLink);
		
		output.deleteTag(eoName + ".mesh");
		output.deleteTag(eoName + ".texture");
		output.deleteTag(eoName + ".mass");
		output.deleteTag(eoName + ".pos");
		output.deleteTag(eoName + ".rot");
		output.deleteTag(eoName + ".scale");
		
		output.addTag(eoName + ".mesh", meshName);
		output.addTag(eoName + ".texture", textureName);
		output.addTag(eoName + ".mass", massStr);
		output.addTag(eoName + ".pos", posLink);
		output.addTag(eoName + ".rot", rotLink);
		output.addTag(eoName + ".scale", scaleLink);
	}
	output.addSequence("objects", objectList);
	output.addSequence("entityobjects", eoList);
}

void World::setGravity(Vector3F gravity)
{
	this->gravity = gravity;
	for(unsigned int i = 0; i < this->getEntities().size(); i++)
	{
		std::cout << i << ".\n";
		Entity* ent = this->getEntities().at(i);
		ent->removeForce("gravity");
		ent->applyForce("gravity", Force(this->getGravity()));
	}
	for(unsigned int i = 0; i < this->getEntityObjects().size(); i++)
	{
		std::shared_ptr<EntityObject> eo = this->getEntityObjects().at(i);
		eo->removeForce("gravity");
		eo->applyForce("gravity", Force(this->getGravity()));
	}
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

std::vector<Entity*> World::getEntities()
{
	return this->entities;
}

std::vector<std::shared_ptr<EntityObject>> World::getEntityObjects()
{
	return this->entityObjects;
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

std::shared_ptr<EntityObject> World::retrieveEOData(std::string eoName, MDLF& mdlf)
{
	std::string meshName = mdlf.getTag(eoName + ".mesh");
	std::string textureName = mdlf.getTag(eoName + ".texture");
	std::string massStr = mdlf.getTag(eoName + ".mass");
	std::string positionStr = mdlf.getTag(eoName + ".pos");
	std::string rotationStr = mdlf.getTag(eoName + ".rot");
	std::string scaleStr = mdlf.getTag(eoName + ".scale");
	
	std::cout << eoName << ".mesh: " << meshName << "\n";
	std::cout << eoName << ".texture: " << textureName << "\n";
	std::cout << eoName << ".mass: " << massStr << "\n";
	std::cout << eoName << ".pos: " << positionStr << "\n";
	std::cout << eoName << ".rot: " << rotationStr << "\n";
	std::cout << eoName << ".scale: " << scaleStr << "\n";
	
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
	
	float mass = CastUtility::fromString<float>(massStr);
	
	return std::shared_ptr<EntityObject>(new EntityObject(meshLink, textureLink, mass, pos, rot, scale));
}