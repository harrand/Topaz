#include "datatranslation.hpp"

//TEMP
#include <iostream>

DataTranslation::DataTranslation(std::string datafilename)
{
	this->datafilename = datafilename;
}

std::string DataTranslation::getMeshLink(std::string meshName)
{
	return FileUtility::getTag(File(this->datafilename), (meshName + ".path"));
}

std::string DataTranslation::getTextureLink(std::string textureName)
{
	return FileUtility::getTag(File(this->datafilename), (textureName + ".path"));
}

std::string DataTranslation::getMeshName(std::string meshLink)
{
	File input(this->datafilename);
	for(unsigned int i = 0; i < (input.lineCount() - 1); i++)
	{
		std::string line = input.getFromLine(i + 1);
		std::vector<std::string> lineSplit = StringUtility::splitString(line, ':');
		if(lineSplit.size() != 0)
		{
			std::string tagname = lineSplit.at(0);
			if(FileUtility::getTag(input, tagname) == meshLink)
			{
				return tagname;
			}
		}
	}
	return "_";
}

std::string DataTranslation::getTextureName(std::string textureLink)
{
	File input(this->datafilename);
	for(unsigned int i = 0; i < (input.lineCount() - 1); i++)
	{
		std::string line = input.getFromLine(i + 1);
		std::vector<std::string> lineSplit = StringUtility::splitString(line, ':');
		if(lineSplit.size() != 0)
		{
			std::string tagname = lineSplit.at(0);
			if(FileUtility::getTag(input, tagname) == textureLink)
			{
				return tagname;
			}
		}
	}
	// There was no such name referenced...
	return "_";
}
// std::map<std::string, std::string> retrieveModels(std::string datafilename);
// std::map<std::string, std::string> retrieveTextures(std::string datafilename);

std::map<std::string, std::string> DataTranslation::retrieveModels()
{
	std::map<std::string, std::string> modelMap;
	File input(this->datafilename);
	std::vector<std::string> models;
	std::string modelListStr = FileUtility::getTag(input, "models");
	std::vector<std::string> modelList;
	if(StringUtility::contains(modelListStr, ','))
		modelList = StringUtility::splitString(modelListStr, ',');
	else if(modelListStr != "_")
		modelList.push_back(modelListStr);
	for(unsigned int i = 0; i < modelList.size(); i++)
	{
		std::string model = modelList.at(i);
		// map is like this: modelList(path, name)
		modelMap[this->getMeshLink(model)] = FileUtility::getTag(input, model + ".name");
	}
	return modelMap;
}

std::map<std::string, std::string> DataTranslation::retrieveTextures()
{
	std::map<std::string, std::string> textureMap;
	File input(this->datafilename);
	std::vector<std::string> textures;
	std::string textureListStr = FileUtility::getTag(input, "textures");
	std::vector<std::string> textureList;
	if(StringUtility::contains(textureListStr, ','))
		textureList = StringUtility::splitString(textureListStr, ',');
	else if(textureListStr != "_")
		textureList.push_back(textureListStr);
	for(unsigned int i = 0; i < textureList.size(); i++)
	{
		std::string texture = textureList.at(i);
		// map is like this: textureList(path, name)
		textureMap[this->getTextureLink(texture)] = FileUtility::getTag(input, texture + ".name");
	}
	return textureMap;
}
