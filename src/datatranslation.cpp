#include "datatranslation.hpp"

DataTranslation::DataTranslation(std::string datafilename): datafilename(datafilename){}

std::string DataTranslation::getMeshLink(std::string meshName)
{
	return MDLF(RawFile(this->datafilename)).getTag(meshName + ".path");
}

std::string DataTranslation::getTextureLink(std::string textureName)
{
	return MDLF(RawFile(this->datafilename)).getTag(textureName + ".path");
}

std::string DataTranslation::getMeshName(std::string meshLink)
{
	const RawFile input(this->datafilename);
	for(unsigned int i = 0; i < input.getLines().size(); i++)
	{
		std::string line = input.getLineByNumber(i);
		std::vector<std::string> lineSplit = StringUtility::splitString(line, ':');
		if(lineSplit.size() != 0)
		{
			std::string tagname = lineSplit.at(0);
			if(MDLF(input).getTag(tagname) == meshLink)
			{
				std::string undesiredSuffix = ".path";
				tagname.erase(tagname.find(undesiredSuffix), undesiredSuffix.length());
				return tagname;
			}
		}
	}
	return "_";
}

std::string DataTranslation::getTextureName(std::string textureLink)
{
	const RawFile input(this->datafilename);
	for(unsigned int i = 0; i < input.getLines().size(); i++)
	{
		std::string line = input.getLineByNumber(i);
		std::vector<std::string> lineSplit = StringUtility::splitString(line, ':');
		if(lineSplit.size() != 0)
		{
			std::string tagname = lineSplit.at(0);
			if(MDLF(input).getTag(tagname) == textureLink)
			{
				std::string undesiredSuffix = ".path";
				tagname.erase(tagname.find(undesiredSuffix), undesiredSuffix.length());
				return tagname;
			}
		}
	}
	return "_";
}

std::map<std::string, std::string> DataTranslation::retrieveModels()
{
	std::map<std::string, std::string> modelMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> modelList = input.getSequence("models");
	for(unsigned int i = 0; i < modelList.size(); i++)
	{
		std::string model = modelList.at(i);
		modelMap[this->getMeshLink(model)] = input.getTag(model + ".name");
	}
	return modelMap;
}

std::map<std::string, std::string> DataTranslation::retrieveTextures()
{
	std::map<std::string, std::string> textureMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> textureList = input.getSequence("textures");
	for(unsigned int i = 0; i < textureList.size(); i++)
	{
		std::string texture = textureList.at(i);
		textureMap[this->getTextureLink(texture)] = input.getTag(texture + ".name");
	}
	return textureMap;
}
