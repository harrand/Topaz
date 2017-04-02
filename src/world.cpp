#include "world.hpp"

World::World(std::string filename): filename(filename)
{
	MDLF input(RawFile(this->filename));
	std::string spawnPointStr = input.getTag("spawnpoint"), spawnOrientationStr = input.getTag("spawnorientation"), gravStr = input.getTag("gravity");
	if(spawnPointStr != "0" && spawnOrientationStr != "0" && gravStr != "0")
	{
		std::vector<std::string> spawnPointSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(spawnPointStr, '[', ""), ']', ""), ',');
		std::vector<std::string> spawnOrientationSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(spawnOrientationStr, '[', ""), ']', ""), ',');
		std::vector<std::string> gravSplit = StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(gravStr, '[', ""), ']', ""), ',');
		this->spawnPoint = Vector3F(CastUtility::fromString<float>(spawnPointSplit.at(0)), CastUtility::fromString<float>(spawnPointSplit.at(1)), CastUtility::fromString<float>(spawnPointSplit.at(2)));
		this->spawnOrientation = Vector3F(CastUtility::fromString<float>(spawnOrientationSplit.at(0)), CastUtility::fromString<float>(spawnOrientationSplit.at(1)), CastUtility::fromString<float>(spawnOrientationSplit.at(2)));
		this->gravity = Vector3F(CastUtility::fromString<float>(gravSplit.at(0)), CastUtility::fromString<float>(gravSplit.at(1)), CastUtility::fromString<float>(gravSplit.at(2)));
	}
	else
	{
		this->spawnPoint = Vector3F();
		this->spawnOrientation = Vector3F();
		this->gravity = Vector3F();
	}

	std::vector<std::string> objectList = input.getSequence("objects");
	std::vector<std::string> entityObjectList = input.getSequence("entityobjects");
	for(unsigned int i = 0; i < objectList.size(); i++)
	{
		std::string objectName = objectList.at(i);
		Object obj = World::retrieveData(objectName, input);
		this->addObject(obj);
	}
	for(unsigned int i = 0; i < entityObjectList.size(); i++)
	{
		std::string eoName = entityObjectList.at(i);
		std::shared_ptr<EntityObject> eo = World::retrieveEOData(eoName, input);
		this->addEntityObject(eo);
	}
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

void World::exportWorld(const std::string& worldName) const
{
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string worldLink = RES_POINT + "/worlds/" + worldName;
	MDLF output = MDLF(RawFile(worldLink));
	output.getRawFile().clear();
	std::vector<std::string> objectList;
	std::vector<std::string> eoList;
	output.deleteSequence("objects");
	output.deleteSequence("entityobjects");
	std::string gravLink = "[" + CastUtility::toString(this->gravity.getX()) + ", " + CastUtility::toString(this->gravity.getY()) + ", " + CastUtility::toString(this->gravity.getZ()) + "]";
	std::string spawnPointLink = "[" + CastUtility::toString(this->spawnPoint.getX()) + ", " + CastUtility::toString(this->spawnPoint.getY()) + ", " + CastUtility::toString(this->spawnPoint.getZ()) + "]";
	std::string spawnOrientationLink = "[" + CastUtility::toString(this->spawnOrientation.getX()) + ", " + CastUtility::toString(this->spawnOrientation.getY()) + ", " + CastUtility::toString(this->spawnOrientation.getZ()) + "]";

	output.editTag("gravity", gravLink);
	output.editTag("spawnpoint", spawnPointLink);
	output.editTag("spawnorientation", spawnOrientationLink);
	for(unsigned int i = 0; i < this->members.size(); i++)
	{
		std::string objectName = "object" + CastUtility::toString(i);
		objectList.push_back(objectName);
		Object curObj = this->members.at(i);
		std::string meshLink = curObj.getMeshLink();
		std::string textureLink = curObj.getTextureLink();
		std::string normalMapLink = curObj.getNormalMapLink();
		std::string parallaxMapLink = curObj.getParallaxMapLink();
		
		Vector3F pos = curObj.getPos(), rot = curObj.getRot(), scale = curObj.getScale();
		
		std::string posLink = "[" + CastUtility::toString(pos.getX()) + ", " + CastUtility::toString(pos.getY()) + ", " + CastUtility::toString(pos.getZ()) + "]";
		std::string rotLink = "[" + CastUtility::toString(rot.getX()) + ", " + CastUtility::toString(rot.getY()) + ", " + CastUtility::toString(rot.getZ()) + "]";
		std::string scaleLink = "[" + CastUtility::toString(scale.getX()) + ", " + CastUtility::toString(scale.getY()) + ", " + CastUtility::toString(scale.getZ()) + "]";
		
		std::string meshName = dt.getResourceName(meshLink);
		std::string textureName = dt.getResourceName(textureLink);
		std::string normalMapName = dt.getResourceName(normalMapLink);
		std::string parallaxMapName = dt.getResourceName(parallaxMapLink);
		
		output.editTag(objectName + ".mesh", meshName);
		output.editTag(objectName + ".texture", textureName);
		output.editTag(objectName + ".normalmap", normalMapName);
		output.editTag(objectName + ".parallaxmap", parallaxMapName);
		output.editTag(objectName + ".pos", posLink);
		output.editTag(objectName + ".rot", rotLink);
		output.editTag(objectName + ".scale", scaleLink);
	}
	for(unsigned int i = 0; i < this->entityObjects.size(); i++)
	{
		std::string eoName = "eo" + CastUtility::toString(i);
		eoList.push_back(eoName);
		std::shared_ptr<EntityObject> curEO = this->entityObjects.at(i);
		std::string meshLink = curEO->getMeshLink();
		std::string textureLink = curEO->getTextureLink();
		std::string normalMapLink = curEO->getNormalMapLink();
		std::string parallaxMapLink = curEO->getParallaxMapLink();
		
		std::string massStr = CastUtility::toString(curEO->getMass());
		
		Vector3F pos = curEO->getPos(), rot = curEO->getRot(), scale = curEO->getScale();
		
		std::string posLink = "[" + CastUtility::toString(pos.getX()) + ", " + CastUtility::toString(pos.getY()) + ", " + CastUtility::toString(pos.getZ()) + "]";
		std::string rotLink = "[" + CastUtility::toString(rot.getX()) + ", " + CastUtility::toString(rot.getY()) + ", " + CastUtility::toString(rot.getZ()) + "]";
		std::string scaleLink = "[" + CastUtility::toString(scale.getX()) + ", " + CastUtility::toString(scale.getY()) + ", " + CastUtility::toString(scale.getZ()) + "]";
		
		std::string meshName = dt.getResourceName(meshLink);
		std::string textureName = dt.getResourceName(textureLink);
		std::string normalMapName = dt.getResourceName(normalMapLink);
		std::string parallaxMapName = dt.getResourceName(parallaxMapLink);
		
		output.editTag(eoName + ".mesh", meshName);
		output.editTag(eoName + ".texture", textureName);
		output.editTag(eoName + ".normalmap", normalMapName);
		output.editTag(eoName + ".parallaxmap", parallaxMapName);
		output.editTag(eoName + ".mass", massStr);
		output.editTag(eoName + ".pos", posLink);
		output.editTag(eoName + ".rot", rotLink);
		output.editTag(eoName + ".scale", scaleLink);
	}
	output.addSequence("objects", objectList);
	output.addSequence("entityobjects", eoList);
}

void World::setGravity(Vector3F gravity)
{
	this->gravity = gravity;
	for(unsigned int i = 0; i < this->getEntities().size(); i++)
	{
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

void World::update(unsigned int fps, Camera& cam, Shader& shader, unsigned int width, unsigned int height, std::vector<std::shared_ptr<Mesh>> allMeshes, std::vector<std::shared_ptr<Texture>> allTextures, std::vector<std::shared_ptr<NormalMap>> allNormalMaps, std::vector<std::shared_ptr<ParallaxMap>> allParallaxMaps) const
{
	for(unsigned int i = 0; i < this->getMembers().size(); i++)
	{
		Object obj = this->getMembers().at(i);
		obj.render(Mesh::getFromLink(obj.getMeshLink(), allMeshes), Texture::getFromLink(obj.getTextureLink(), allTextures), NormalMap::getFromLink(obj.getNormalMapLink(), allNormalMaps), ParallaxMap::getFromLink(obj.getParallaxMapLink(), allParallaxMaps), cam, shader, width, height);
	}
		
	for(unsigned int i = 0; i < this->getEntityObjects().size(); i++)
	{
		std::shared_ptr<EntityObject> eo = this->getEntityObjects().at(i);
		eo->render(Mesh::getFromLink(eo->getMeshLink(), allMeshes), Texture::getFromLink(eo->getTextureLink(), allTextures), NormalMap::getFromLink(eo->getNormalMapLink(), allNormalMaps), ParallaxMap::getFromLink(eo->getParallaxMapLink(), allParallaxMaps), cam, shader, width, height);
		eo->updateMotion(fps);
	}
		
	for(unsigned int i = 0; i < this->getEntities().size(); i++)
	{
		Entity* ent = this->getEntities().at(i);
		ent->updateMotion(fps);
	}
}

unsigned int World::getSize() const
{
	return this->members.size();
}

std::vector<Object> World::getMembers() const
{
	return this->members;
}

std::vector<Entity*> World::getEntities() const
{
	return this->entities;
}

std::vector<std::shared_ptr<EntityObject>> World::getEntityObjects() const
{
	return this->entityObjects;
}

Vector3F World::getGravity() const
{
	return this->gravity;
}

Vector3F World::getSpawnPoint() const
{
	return this->spawnPoint;
}

Vector3F World::getSpawnOrientation() const
{
	return this->spawnOrientation;
}

std::string World::getWorldLink() const
{
	return this->filename;
}

Object World::retrieveData(const std::string& objectName, MDLF& mdlf)
{
	std::string meshName = mdlf.getTag(objectName + ".mesh");
	std::string textureName = mdlf.getTag(objectName + ".texture");
	std::string normalMapName = mdlf.getTag(objectName + ".normalmap");
	std::string parallaxMapName = mdlf.getTag(objectName + ".parallaxmap");
	std::string positionStr = mdlf.getTag(objectName + ".pos");
	std::string rotationStr = mdlf.getTag(objectName + ".rot");
	std::string scaleStr = mdlf.getTag(objectName + ".scale");
	
	DataTranslation dt(RES_POINT + "/resources.data");
	
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	std::string parallaxMapLink = dt.getResourceLink(parallaxMapName);
	
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
	
	return Object(meshLink, textureLink, normalMapLink, parallaxMapLink, pos, rot, scale);
}

std::shared_ptr<EntityObject> World::retrieveEOData(const std::string& eoName, MDLF& mdlf)
{
	std::string meshName = mdlf.getTag(eoName + ".mesh");
	std::string textureName = mdlf.getTag(eoName + ".texture");
	std::string normalMapName = mdlf.getTag(eoName + ".normalmap");
	std::string parallaxMapName = mdlf.getTag(eoName + ".parallaxmap");
	std::string massStr = mdlf.getTag(eoName + ".mass");
	std::string positionStr = mdlf.getTag(eoName + ".pos");
	std::string rotationStr = mdlf.getTag(eoName + ".rot");
	std::string scaleStr = mdlf.getTag(eoName + ".scale");
	
	DataTranslation dt(RES_POINT + "/resources.data");
	
	std::string meshLink = dt.getResourceLink(meshName);
	std::string textureLink = dt.getResourceLink(textureName);
	std::string normalMapLink = dt.getResourceLink(normalMapName);
	std::string parallaxMapLink = dt.getResourceLink(parallaxMapName);
	
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
	
	return std::shared_ptr<EntityObject>(new EntityObject(meshLink, textureLink, normalMapLink, parallaxMapLink, mass, pos, rot, scale));
}