#include "world.hpp"

//template
#include <iostream>

World::World(std::string filename)
{
	std::cout << "loading new world...\n";
	this->filename = filename;
	this->allocatedHeap = false;
	File input(filename);
	std::string objectList = FileUtility::getTag(input, "objects");
	std::vector<std::string> objectListV;
	if(StringUtility::contains(objectList, ','))
		objectListV = StringUtility::splitString(objectList, ',');
	else if(objectList != "_")
		objectListV.push_back(objectList);
	for(unsigned int i = 0; i < objectListV.size(); i++)
	{
		std::string objectName = objectListV.at(i);
		Object obj = this->retrieveData(objectName, input);
		this->addObject(obj);
		std::cout << "Loaded " << objectName << " into world.\n";
		std::cout << objectName << " uses the Mesh '" << obj.getMeshLink() << "' and the Texture '" << obj.getTextureLink() << "'.\n";
	}
}

World::World(Object* objects, unsigned int numObjects)
{
	for(unsigned int i = 0; i < numObjects; i++)
	{
		members.push_back(objects[i]);
	}
	this->allocatedHeap = false;
}

World::~World()
{
	/*
	if(allocatedHeap)
	{
		for(unsigned int i = 0; i < meshHeap.size(); i++)
			delete this->meshHeap.at(i);
		for(unsigned int i = 0; i < textureHeap.size(); i++)
			delete this->textureHeap.at(i);
	}]
	*/
}

void World::addObject(Object obj)
{
	this->members.push_back(obj);
}

void World::exportWorld(std::string worldName)
{
	DataTranslation dt("./res/data/resourcelist.data");
	std::string worldLink = "./res/data/worlds/" + worldName;
	File output(worldLink);
	if(!output.exists())
	{
		output = FileUtility::createRelativeFile(worldLink);
	}
	output.clear();
	for(unsigned int i = 0; i < this->members.size(); i++)
	{
		Object curObj = this->members.at(i);
		std::string objectName = "object" + StringUtility::toString(i);
		std::string objectList = FileUtility::getTag(output, "objects");
		if(objectList == "_")
			objectList = "";
		objectList += (objectList == "") ? (objectName) : ("," + objectName);
		FileUtility::setTag(output, "objects", objectList);
				
		std::string meshLink = curObj.getMeshLink();
		std::string textureLink = curObj.getTextureLink();
		Vector3F pos = curObj.getPos();
		Vector3F rot = curObj.getRot();
		Vector3F scale = curObj.getScale();
				
		std::string posLink = "[" + StringUtility::toString(pos.getX()) + ", " + StringUtility::toString(pos.getY()) + ", " + StringUtility::toString(pos.getZ()) + "]";
		std::string rotLink = "[" + StringUtility::toString(rot.getX()) + ", " + StringUtility::toString(rot.getY()) + ", " + StringUtility::toString(rot.getZ()) + "]";
		std::string scaleLink = "[" + StringUtility::toString(scale.getX()) + ", " + StringUtility::toString(scale.getY()) + ", " + StringUtility::toString(scale.getZ()) + "]";
		
		std::string meshName = dt.getMeshName(meshLink);
		std::string textureName = dt.getTextureName(textureLink);
				
		FileUtility::setTag(output, (objectName + ".mesh"), meshName);
		FileUtility::setTag(output, (objectName + ".texture"), textureName);
		FileUtility::setTag(output, (objectName + ".pos"), posLink);
		FileUtility::setTag(output, (objectName + ".rot"), rotLink);
		FileUtility::setTag(output, (objectName + ".scale"), scaleLink);		
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

Object World::retrieveData(std::string objectName, File f)
{
	DataTranslation dt("./res/resources.data");
	std::string meshName = FileUtility::getTag(f, (objectName + ".mesh"));
	std::string textureName = FileUtility::getTag(f, (objectName + ".texture"));
	std::string positionStr = FileUtility::getTag(f, (objectName + ".pos"));
	std::string rotationStr = FileUtility::getTag(f, (objectName + ".rot"));
	std::string scaleStr = FileUtility::getTag(f, (objectName + ".scale"));
	
	//Mesh mesh(dt.getMeshLink(meshName));
	//Texture texture(dt.getTextureLink(textureName));
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
	
	/*
	// Keep track of the heap-allocated objects. Garbage collection occurs at destructor.
	this->meshHeap.push_back(mesh);
	this->textureHeap.push_back(texture);
	*/
	
	return Object(meshLink, textureLink, pos, rot, scale);
}