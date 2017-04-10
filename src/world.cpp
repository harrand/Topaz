#include "world.hpp"

World::World(std::string filename): filename(filename)
{
	MDLF input(RawFile(this->filename));
	std::string spawnPointStr = input.getTag("spawnpoint"), spawnOrientationStr = input.getTag("spawnorientation"), gravStr = input.getTag("gravity");
	if(spawnPointStr != "0" && spawnOrientationStr != "0" && gravStr != "0")
	{
		this->spawnPoint = StringUtility::vectoriseList3F(StringUtility::deformat(spawnPointStr));
		this->spawnOrientation = StringUtility::vectoriseList3F(StringUtility::deformat(spawnOrientationStr));
		this->gravity = StringUtility::vectoriseList3F(StringUtility::deformat(gravStr));
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
		this->addEntityObject(World::retrieveEOData(eoName, input));
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

void World::addEntityObject(std::unique_ptr<EntityObject>&& eo)
{
	if(eo->getForces().find("gravity") != eo->getForces().end())
	{
		eo->removeForce("gravity");
	}
	eo->applyForce("gravity", Force(this->getGravity()));
	this->entityObjects.push_back(std::move(eo));
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
	
	output.editTag("gravity", StringUtility::format(StringUtility::devectoriseList3F(this->gravity)));
	output.editTag("spawnpoint", StringUtility::format(StringUtility::devectoriseList3F(this->spawnPoint)));
	output.editTag("spawnorientation", StringUtility::format(StringUtility::devectoriseList3F(this->spawnOrientation)));
	for(unsigned int i = 0; i < this->members.size(); i++)
	{
		std::string objectName = "object" + CastUtility::toString<float>(i);
		objectList.push_back(objectName);
		Object curObj = this->members.at(i);
		
		output.editTag(objectName + ".mesh", dt.getResourceName(curObj.getMeshLink()));
		output.editTag(objectName + ".texture", dt.getResourceName(curObj.getTextureLink()));
		output.editTag(objectName + ".normalmap", dt.getResourceName(curObj.getNormalMapLink()));
		output.editTag(objectName + ".parallaxmap", dt.getResourceName(curObj.getParallaxMapLink()));
		output.editTag(objectName + ".pos", StringUtility::format(StringUtility::devectoriseList3F(curObj.getPos())));
		output.editTag(objectName + ".rot", StringUtility::format(StringUtility::devectoriseList3F(curObj.getRot())));
		output.editTag(objectName + ".scale", StringUtility::format(StringUtility::devectoriseList3F(curObj.getScale())));
	}
	for(unsigned int i = 0; i < this->entityObjects.size(); i++)
	{
		std::string eoName = "eo" + CastUtility::toString<float>(i);
		eoList.push_back(eoName);
		EntityObject* curEO = this->entityObjects.at(i).get();

		output.editTag(eoName + ".mesh", dt.getResourceName(curEO->getMeshLink()));
		output.editTag(eoName + ".texture", dt.getResourceName(curEO->getTextureLink()));
		output.editTag(eoName + ".normalmap", dt.getResourceName(curEO->getNormalMapLink()));
		output.editTag(eoName + ".parallaxmap", dt.getResourceName(curEO->getParallaxMapLink()));
		output.editTag(eoName + ".mass", CastUtility::toString<float>(curEO->getMass()));
		output.editTag(eoName + ".pos", StringUtility::format(StringUtility::devectoriseList3F(curEO->getPos())));
		output.editTag(eoName + ".rot", StringUtility::format(StringUtility::devectoriseList3F(curEO->getRot())));
		output.editTag(eoName + ".scale", StringUtility::format(StringUtility::devectoriseList3F(curEO->getScale())));
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
		EntityObject* eo = this->getEntityObjects().at(i).get();
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

void World::update(unsigned int fps, Camera& cam, Shader& shader, unsigned int width, unsigned int height, const std::vector<std::unique_ptr<Mesh>>& allMeshes, const std::vector<std::unique_ptr<Texture>>& allTextures, const std::vector<std::unique_ptr<NormalMap>>& allNormalMaps, const std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps) const
{
	for(unsigned int i = 0; i < this->getMembers().size(); i++)
	{
		Object obj = this->getMembers().at(i);
		obj.render(Mesh::getFromLink(obj.getMeshLink(), allMeshes), Texture::getFromLink(obj.getTextureLink(), allTextures), NormalMap::getFromLink(obj.getNormalMapLink(), allNormalMaps), ParallaxMap::getFromLink(obj.getParallaxMapLink(), allParallaxMaps), cam, shader, width, height);
	}
		
	for(unsigned int i = 0; i < this->getEntityObjects().size(); i++)
	{
		EntityObject* eo = this->getEntityObjects().at(i).get();
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
	return this->members.size() + this->entities.size() + this->entityObjects.size();
}

const std::vector<Object>& World::getMembers() const
{
	return this->members;
}

const std::vector<Entity*>& World::getEntities() const
{
	return this->entities;
}

const std::vector<std::unique_ptr<EntityObject>>& World::getEntityObjects() const
{
	return this->entityObjects;
}

const Vector3F& World::getGravity() const
{
	return this->gravity;
}

const Vector3F& World::getSpawnPoint() const
{
	return this->spawnPoint;
}

const Vector3F& World::getSpawnOrientation() const
{
	return this->spawnOrientation;
}

const std::string& World::getWorldLink() const
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
	
	return Object(meshLink, textureLink, normalMapLink, parallaxMapLink, StringUtility::vectoriseList3F(StringUtility::deformat(positionStr)), StringUtility::vectoriseList3F(StringUtility::deformat(rotationStr)), StringUtility::vectoriseList3F(StringUtility::deformat(scaleStr)));
}

std::unique_ptr<EntityObject> World::retrieveEOData(const std::string& eoName, MDLF& mdlf)
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
	float mass = CastUtility::fromString<float>(massStr);
	
	return std::make_unique<EntityObject>(meshLink, textureLink, normalMapLink, parallaxMapLink, mass, StringUtility::vectoriseList3F(StringUtility::deformat(positionStr)), StringUtility::vectoriseList3F(StringUtility::deformat(rotationStr)), StringUtility::vectoriseList3F(StringUtility::deformat(scaleStr)));
}