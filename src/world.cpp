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
}

World::World(Object* objects, unsigned int numObjects)
{
	for(unsigned int i = 0; i < numObjects; i++)
	{
		members.push_back(objects[i]);
	}
}

void World::addObject(Object obj)
{
	this->members.push_back(obj);
}

void World::exportWorld(std::string worldName)
{
	DataTranslation dt(RES_POINT + "/resources.data");
	std::string worldLink = RES_POINT + "/data/worlds/" + worldName;
	MDLF output = MDLF(RawFile(worldLink));
	output.getRawFile().clear();
	for(unsigned int i = 0; i < this->members.size(); i++)
	{
		Object curObj = this->members.at(i);
		std::string objectName = "object" + StringUtility::toString(i);
		std::vector<std::string> objectList = output.getSequence("objects");
		objectList.push_back(objectName);
		
		output.deleteSequence("objects");
		output.addSequence("objects", objectList);
		std::string meshLink = curObj.getMeshLink();
		std::string textureLink = curObj.getTextureLink();
		
		Vector3F pos = curObj.getPos(), rot = curObj.getRot(), scale = curObj.getScale();
		
		std::string posLink = "[" + StringUtility::toString(pos.getX()) + ", " + StringUtility::toString(pos.getY()) + ", " + StringUtility::toString(pos.getZ()) + "]";
		std::string rotLink = "[" + StringUtility::toString(rot.getX()) + ", " + StringUtility::toString(rot.getY()) + ", " + StringUtility::toString(rot.getZ()) + "]";
		std::string scaleLink = "[" + StringUtility::toString(scale.getX()) + ", " + StringUtility::toString(scale.getY()) + ", " + StringUtility::toString(scale.getZ()) + "]";
		
		std::string meshName = dt.getMeshName(meshLink);
		std::string textureName = dt.getTextureName(textureLink);
		
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
}

unsigned int World::getSize()
{
	return this->members.size();
}

std::vector<Object> World::getMembers()
{
	return this->members;
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
	
	std::string meshLink = dt.getMeshLink(meshName);
	std::string textureLink = dt.getTextureLink(textureName);
	
	std::vector<std::string> posData = StringUtility::splitString(StringUtility::replaceAllChar((StringUtility::replaceAllChar(positionStr, '[', "")), ']', ""), ',');
	float posX = MathsUtility::parseTemplateFloat(posData.at(0));
	float posY = MathsUtility::parseTemplateFloat(posData.at(1));
	float posZ = MathsUtility::parseTemplateFloat(posData.at(2));
	Vector3F pos(posX, posY, posZ);
	
	std::vector<std::string> rotData = StringUtility::splitString(StringUtility::replaceAllChar((StringUtility::replaceAllChar(rotationStr, '[', "")), ']', ""), ',');
	float rotX = MathsUtility::parseTemplateFloat(rotData.at(0));
	float rotY = MathsUtility::parseTemplateFloat(rotData.at(1));
	float rotZ = MathsUtility::parseTemplateFloat(rotData.at(2));
	Vector3F rot(rotX, rotY, rotZ);
	
	std::vector<std::string> scaleData = StringUtility::splitString(StringUtility::replaceAllChar((StringUtility::replaceAllChar(scaleStr, '[', "")), ']', ""), ',');
	float scaleX = MathsUtility::parseTemplateFloat(scaleData.at(0));
	float scaleY = MathsUtility::parseTemplateFloat(scaleData.at(1));
	float scaleZ = MathsUtility::parseTemplateFloat(scaleData.at(2));
	Vector3F scale(scaleX, scaleY, scaleZ);
	
	return Object(meshLink, textureLink, pos, rot, scale);
}