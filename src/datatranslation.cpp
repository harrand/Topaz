#include "datatranslation.hpp"

DataTranslation::DataTranslation(std::string datafilename): datafilename(datafilename){}

std::string DataTranslation::getResourceLink(std::string resourceName) const
{
	return MDLF(RawFile(this->datafilename)).getTag(resourceName + ".path");
}

std::string DataTranslation::getResourceName(std::string resourceLink) const
{
	const RawFile input(this->datafilename);
	for(unsigned int i = 0; i < input.getLines().size(); i++)
	{
		std::string line = input.getLineByNumber(i);
		std::vector<std::string> lineSplit = StringUtility::splitString(line, ':');
		if(lineSplit.size() != 0)
		{
			std::string tagname = lineSplit.at(0);
			if(MDLF(input).getTag(tagname) == resourceLink)
			{
				std::string undesiredSuffix = ".path";
				tagname.erase(tagname.find(undesiredSuffix), undesiredSuffix.length());
				return tagname;
			}
		}
	}
	return "0";
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveModels() const
{
	std::unordered_map<std::string, std::string> modelMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> modelList = input.getSequence("models");
	for(unsigned int i = 0; i < modelList.size(); i++)
	{
		std::string model = modelList.at(i);
		modelMap[this->getResourceLink(model)] = input.getTag(model + ".name");
	}
	return modelMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveTextures() const
{
	std::unordered_map<std::string, std::string> textureMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> textureList = input.getSequence("textures");
	for(unsigned int i = 0; i < textureList.size(); i++)
	{
		std::string texture = textureList.at(i);
		textureMap[this->getResourceLink(texture)] = input.getTag(texture + ".name");
	}
	return textureMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveNormalMaps() const
{
	std::unordered_map<std::string, std::string> normalMapMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> normalMapList = input.getSequence("normalmaps");
	for(unsigned int i = 0; i < normalMapList.size(); i++)
	{
		std::string normalmap = normalMapList.at(i);
		normalMapMap[this->getResourceLink(normalmap)] = input.getTag(normalmap + ".name");
	}
	return normalMapMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveParallaxMaps() const
{
	std::unordered_map<std::string, std::string> parallaxMapMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> parallaxMapList = input.getSequence("parallaxmaps");
	for(unsigned int i = 0; i < parallaxMapList.size(); i++)
	{
		std::string parallaxmap = parallaxMapList.at(i);
		parallaxMapMap[this->getResourceLink(parallaxmap)] = input.getTag(parallaxmap + ".name");
	}
	return parallaxMapMap;
}
