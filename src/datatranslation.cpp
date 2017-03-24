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

std::map<std::string, std::string> DataTranslation::retrieveModels() const
{
	std::map<std::string, std::string> modelMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> modelList = input.getSequence("models");
	for(unsigned int i = 0; i < modelList.size(); i++)
	{
		std::string model = modelList.at(i);
		modelMap[this->getResourceLink(model)] = input.getTag(model + ".name");
	}
	return modelMap;
}

std::map<std::string, std::string> DataTranslation::retrieveTextures() const
{
	std::map<std::string, std::string> textureMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> textureList = input.getSequence("textures");
	for(unsigned int i = 0; i < textureList.size(); i++)
	{
		std::string texture = textureList.at(i);
		textureMap[this->getResourceLink(texture)] = input.getTag(texture + ".name");
	}
	return textureMap;
}

std::map<std::string, std::string> DataTranslation::retrieveNormalMaps() const
{
	std::map<std::string, std::string> normalMapMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> normalMapList = input.getSequence("normalmaps");
	for(unsigned int i = 0; i < normalMapList.size(); i++)
	{
		std::string normalmap = normalMapList.at(i);
		normalMapMap[this->getResourceLink(normalmap)] = input.getTag(normalmap + ".name");
	}
	return normalMapMap;
}

std::map<std::string, std::string> DataTranslation::retrieveDisplacementMaps() const
{
	std::map<std::string, std::string> displacementMapMap;
	MDLF input(RawFile(this->datafilename));
	std::vector<std::string> displacementMapList = input.getSequence("displacementmaps");
	for(unsigned int i = 0; i < displacementMapList.size(); i++)
	{
		std::string displacementmap = displacementMapList.at(i);
		displacementMapMap[this->getResourceLink(displacementmap)] = input.getTag(displacementmap + ".name");
	}
	return displacementMapMap;
}
