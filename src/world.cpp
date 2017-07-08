#include "world.hpp"

World::World(std::string filename, std::string resources_path): filename(filename), resources_path(resources_path)
{
	MDLF input(RawFile(this->filename));
	std::string spawnPointStr = input.getTag("spawnpoint"), spawnOrientationStr = input.getTag("spawnorientation"), gravStr = input.getTag("gravity");
	if(spawnPointStr != "0" && spawnOrientationStr != "0" && gravStr != "0")
	{
		this->spawnPoint = StringUtility::vectoriseList3<float>(StringUtility::deformat(spawnPointStr));
		this->spawnOrientation = StringUtility::vectoriseList3<float>(StringUtility::deformat(spawnOrientationStr));
		this->gravity = StringUtility::vectoriseList3<float>(StringUtility::deformat(gravStr));
	}
	else
	{
		this->spawnPoint = Vector3F();
		this->spawnOrientation = Vector3F();
		this->gravity = Vector3F();
	}
	std::vector<std::string> objectList = input.getSequence("objects");
	std::vector<std::string> entityObjectList = input.getSequence("entityobjects");
	for(std::string objectName : objectList)
		this->addObject(World::retrieveData(objectName, resources_path, input));
	for(std::string eoName : entityObjectList)
		this->addEntityObject(World::retrieveEOData(eoName, resources_path, input));
}

World::World(const World& copy): World(copy.filename){}

World::World(World&& move): filename(move.filename), resources_path(move.resources_path), gravity(move.gravity), spawnPoint(move.spawnPoint), spawnOrientation(move.spawnOrientation), objects(move.objects), entities(move.entities), entityObjects(std::move(move.entityObjects)), baseLights(std::move(move.baseLights)){}

World::~World()
{
	this->killLights();
}

const std::string& World::getFileName() const
{
	return this->filename;
}

void World::addObject(Object obj)
{
	this->objects.push_back(std::move(obj));
}

void World::addEntity(Entity ent)
{
	if(ent.getForces().find("gravity") != ent.getForces().end())
	{
		ent.removeForce("gravity");
	}
	ent.applyForce("gravity", Force(this->getGravity()));
	this->entities.push_back(std::move(ent));
}

void World::addEntityObject(EntityObject eo)
{
	if(eo.getForces().find("gravity") != eo.getForces().end())
	{
		eo.removeForce("gravity");
	}
	eo.applyForce("gravity", Force(this->getGravity()));
	this->entityObjects.push_back(std::move(eo));
}

void World::addLight(BaseLight light, GLuint shader_programHandle)
{
	while(this->baseLights.size() >= World::MAXIMUM_LIGHTS)
		this->baseLights.erase(this->baseLights.begin());
	std::vector<GLuint> uniforms;
	uniforms.push_back(glGetUniformLocation(shader_programHandle, ("lights[" + CastUtility::toString<unsigned int>(this->baseLights.size()) + "].pos").c_str()));
	uniforms.push_back(glGetUniformLocation(shader_programHandle, ("lights[" + CastUtility::toString<unsigned int>(this->baseLights.size()) + "].colour").c_str()));
	uniforms.push_back(glGetUniformLocation(shader_programHandle, ("lights[" + CastUtility::toString<unsigned int>(this->baseLights.size()) + "].power").c_str()));
	this->baseLights[uniforms] = light;
}

void World::setGravity(Vector3F gravity)
{
	this->gravity = gravity;
	for(Entity& ent : this->entities)
	{
		ent.removeForce("gravity");
		ent.applyForce("gravity", Force(this->getGravity()));
	}
	for(EntityObject& eo : this->entityObjects)
	{
		eo.removeForce("gravity");
		eo.applyForce("gravity", Force(this->getGravity()));
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

void World::killLights()
{
	for(auto& iter : this->baseLights)
	{
		// Kill all the lights in the shader before losing all the data
		std::vector<float> pos({0.0f, 0.0f, 0.0f}), colour({0.0f, 0.0f, 0.0f});
		glUniform3fv(iter.first.at(0), 1, &(pos[0]));
		glUniform3fv(iter.first.at(1), 1, &(colour[0]));
		glUniform1f(iter.first.at(2), 0);
	}
}

void World::exportWorld(const std::string& worldLink) const
{
	const DataTranslation dt(this->resources_path);
	MDLF output = MDLF(RawFile(worldLink));
	output.getRawFile().clear();
	std::vector<std::string> objectList;
	std::vector<std::string> eoList;
	output.deleteSequence("objects");
	output.deleteSequence("entityobjects");
	
	output.editTag("gravity", StringUtility::format(StringUtility::devectoriseList3<float>(this->gravity)));
	output.editTag("spawnpoint", StringUtility::format(StringUtility::devectoriseList3<float>(this->spawnPoint)));
	output.editTag("spawnorientation", StringUtility::format(StringUtility::devectoriseList3<float>(this->spawnOrientation)));
	for(std::size_t i = 0; i < this->objects.size(); i++)
	{
		const std::string objectName = "object" + CastUtility::toString<float>(i);
		objectList.push_back(objectName);
		const Object curObj = this->objects.at(i);
		
		output.editTag(objectName + ".mesh", dt.getResourceName(curObj.getMeshLink()));
		for(auto& texture : curObj.getTextures())
		{
			output.editTag(objectName + ".texture" + CastUtility::toString<unsigned int>(static_cast<unsigned int>(texture.second)), dt.getResourceName(texture.first));
		}
		output.editTag(objectName + ".pos", StringUtility::format(StringUtility::devectoriseList3<float>(curObj.getPosition())));
		output.editTag(objectName + ".rot", StringUtility::format(StringUtility::devectoriseList3<float>(curObj.getRotation())));
		output.editTag(objectName + ".scale", StringUtility::format(StringUtility::devectoriseList3<float>(curObj.getScale())));
	}
	for(std::size_t i = 0; i < this->entityObjects.size(); i++)
	{
		const std::string eoName = "eo" + CastUtility::toString<float>(i);
		eoList.push_back(eoName);
		const EntityObject curEO = this->entityObjects.at(i);

		output.editTag(eoName + ".mesh", dt.getResourceName(curEO.getMeshLink()));
		for(auto& texture : curEO.getTextures())
		{
			output.editTag(eoName + ".texture" + CastUtility::toString<unsigned int>(static_cast<unsigned int>(texture.second)), dt.getResourceName(texture.first));
		}
		output.editTag(eoName + ".mass", CastUtility::toString<float>(curEO.getMass()));
		output.editTag(eoName + ".pos", StringUtility::format(StringUtility::devectoriseList3<float>(curEO.getPosition())));
		output.editTag(eoName + ".rot", StringUtility::format(StringUtility::devectoriseList3<float>(curEO.getRotation())));
		output.editTag(eoName + ".scale", StringUtility::format(StringUtility::devectoriseList3<float>(curEO.getScale())));
	}
	output.addSequence("objects", objectList);
	output.addSequence("entityobjects", eoList);
}

void World::update(unsigned int fps, Camera& cam, const Shader& shader, unsigned int width, unsigned int height, const std::vector<std::unique_ptr<Mesh>>& allMeshes, const std::vector<std::unique_ptr<Texture>>& allTextures, const std::vector<std::unique_ptr<NormalMap>>& allNormalMaps, const std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps, const std::vector<std::unique_ptr<DisplacementMap>>& allDisplacementMaps)
{
	for(auto& obj : this->objects)
	{
		Mesh* mesh = Mesh::getFromLink(obj.getMeshLink(), allMeshes);
		Texture* tex = nullptr; NormalMap* nm = nullptr; ParallaxMap* pm = nullptr; DisplacementMap* dm = nullptr;
		for(auto& texture : obj.getTextures())
		{
			if(texture.second == Texture::TextureType::TEXTURE)
				tex = Texture::getFromLink<Texture>(texture.first, allTextures);
			else if(texture.second == Texture::TextureType::NORMAL_MAP)
				nm = Texture::getFromLink<NormalMap>(texture.first, allNormalMaps);
			else if(texture.second == Texture::TextureType::PARALLAX_MAP)
				pm = Texture::getFromLink<ParallaxMap>(texture.first, allParallaxMaps);
			else if(texture.second == Texture::TextureType::DISPLACEMENT_MAP)
				dm = Texture::getFromLink<DisplacementMap>(texture.first, allDisplacementMaps);
		}
		obj.render(mesh, tex, nm, pm, dm, cam, shader, width, height);
	}
	for(auto& eo : this->entityObjects)
	{
		Mesh* mesh = Mesh::getFromLink(eo.getMeshLink(), allMeshes);
		Texture* tex = nullptr; NormalMap* nm = nullptr; ParallaxMap* pm = nullptr; DisplacementMap* dm = nullptr;
		for(auto& texture : eo.getTextures())
		{
			if(texture.second == Texture::TextureType::TEXTURE)
				tex = Texture::getFromLink<Texture>(texture.first, allTextures);
			else if(texture.second == Texture::TextureType::NORMAL_MAP)
				nm = Texture::getFromLink<NormalMap>(texture.first, allNormalMaps);
			else if(texture.second == Texture::TextureType::PARALLAX_MAP)
				pm = Texture::getFromLink<ParallaxMap>(texture.first, allParallaxMaps);
			else if(texture.second == Texture::TextureType::DISPLACEMENT_MAP)
				dm = Texture::getFromLink<DisplacementMap>(texture.first, allDisplacementMaps);
		}
		eo.render(mesh, tex, nm, pm, dm, cam, shader, width, height);
		eo.updateMotion(fps);
	}	
	for(auto& ent : this->entities)
		ent.updateMotion(fps);
	for(auto& iter : this->baseLights)
	{
		BaseLight light = iter.second;
		std::vector<float> pos, colour;
		pos.push_back(light.getPosition().getX());
		pos.push_back(light.getPosition().getY());
		pos.push_back(light.getPosition().getZ());
		colour.push_back(light.getColour().getX());
		colour.push_back(light.getColour().getY());
		colour.push_back(light.getColour().getZ());
		glUniform3fv(iter.first.at(0), 1, &(pos[0]));
		glUniform3fv(iter.first.at(1), 1, &(colour[0]));
		glUniform1f(iter.first.at(2), light.getPower());
	}
	/*
	if(this->hasSkybox())
			this->skybox->render(cam, this->skyboxShader.value_or(shader), allMeshes, width, height);
	*/
}

std::size_t World::getSize() const
{
	return this->objects.size() + this->entities.size() + this->entityObjects.size();
}

const std::vector<Object>& World::getObjects() const
{
	return this->objects;
}

const std::vector<Entity>& World::getEntities() const
{
	return this->entities;
}

const std::vector<EntityObject>& World::getEntityObjects() const
{
	return this->entityObjects;
}

std::vector<Object>& World::getObjectsR()
{
	return this->objects;
}

std::vector<Entity>& World::getEntitiesR()
{
	return this->entities;
}

std::vector<EntityObject>& World::getEntityObjectsR()
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

Vector3F& World::getGravityR()
{
	return this->gravity;
}

Vector3F& World::getSpawnPointR()
{
	return this->spawnPoint;
}

Vector3F& World::getSpawnOrientationR()
{
	return this->spawnOrientation;
}

const std::string& World::getWorldLink() const
{
	return this->filename;
}

const std::map<std::vector<GLuint>, BaseLight>& World::getLights() const
{
	return this->baseLights;
}

std::map<std::vector<GLuint>, BaseLight>& World::getLightsR()
{
	return this->baseLights;
}

Object World::retrieveData(const std::string& objectName, std::string resources_path, MDLF& mdlf)
{
	std::string meshName = mdlf.getTag(objectName + ".mesh");

	std::string positionStr = mdlf.getTag(objectName + ".pos");
	std::string rotationStr = mdlf.getTag(objectName + ".rot");
	std::string scaleStr = mdlf.getTag(objectName + ".scale");
	
	DataTranslation dt(resources_path);
	
	std::string meshLink = dt.getResourceLink(meshName);
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	for(unsigned int i = 0; i < static_cast<unsigned int>(Texture::TextureType::TEXTURE_TYPES); i++)
	{
		std::string textureName = mdlf.getTag(objectName + ".texture" + CastUtility::toString<unsigned int>(i));
		std::string textureLink = dt.getResourceLink(textureName);
		textures.push_back(std::make_pair(textureLink, static_cast<Texture::TextureType>(i)));
	}
	
	return Object(meshLink, textures, StringUtility::vectoriseList3<float>(StringUtility::deformat(positionStr)), StringUtility::vectoriseList3<float>(StringUtility::deformat(rotationStr)), StringUtility::vectoriseList3<float>(StringUtility::deformat(scaleStr)));
}

EntityObject World::retrieveEOData(const std::string& eoName, std::string resources_path, MDLF& mdlf)
{
	std::string meshName = mdlf.getTag(eoName + ".mesh");
	std::string massStr = mdlf.getTag(eoName + ".mass");
	std::string positionStr = mdlf.getTag(eoName + ".pos");
	std::string rotationStr = mdlf.getTag(eoName + ".rot");
	std::string scaleStr = mdlf.getTag(eoName + ".scale");
	
	DataTranslation dt(resources_path);
	
	std::string meshLink = dt.getResourceLink(meshName);
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	for(unsigned int i = 0; i < static_cast<unsigned int>(Texture::TextureType::TEXTURE_TYPES); i++)
	{
		std::string textureName = mdlf.getTag(eoName + ".texture" + CastUtility::toString<unsigned int>(i));
		std::string textureLink = dt.getResourceLink(textureName);
		//LogUtility::message(eoName, ".texture", CastUtility::toString<unsigned int>(i), " yields the name ", textureName, " and the link ", textureLink, "\n");
		textures.push_back(std::make_pair(textureLink, static_cast<Texture::TextureType>(i)));
	}
	float mass = CastUtility::fromString<float>(massStr);
	
	return EntityObject(meshLink, textures, mass, StringUtility::vectoriseList3<float>(StringUtility::deformat(positionStr)), StringUtility::vectoriseList3<float>(StringUtility::deformat(rotationStr)), StringUtility::vectoriseList3<float>(StringUtility::deformat(scaleStr)));
}