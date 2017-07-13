#include "datatranslation.hpp"
#include "utility.hpp"

DataTranslation::DataTranslation(std::string datafilename): datafilename(std::move(datafilename)){}

std::string DataTranslation::getResourceLink(const std::string& resourceName) const
{
	return MDLF(RawFile(this->datafilename)).getTag(resourceName + ".path");
}

std::string DataTranslation::getResourceName(const std::string& resourceLink) const
{
	const RawFile input(this->datafilename);
	std::vector<std::string> lines = input.getLines();
	for(std::string& line : lines)
	{
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
	for(std::string& model : input.getSequence("models"))
	{
		modelMap[this->getResourceLink(model)] = input.getTag(model + ".name");
	}
	return modelMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveTextures() const
{
	std::unordered_map<std::string, std::string> textureMap;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& texture : input.getSequence("textures"))
	{
		textureMap[this->getResourceLink(texture)] = input.getTag(texture + ".name");
	}
	return textureMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveNormalMaps() const
{
	std::unordered_map<std::string, std::string> normalMapMap;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& normalmap : input.getSequence("normalmaps"))
	{
		normalMapMap[this->getResourceLink(normalmap)] = input.getTag(normalmap + ".name");
	}
	return normalMapMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveParallaxMaps() const
{
	std::unordered_map<std::string, std::string> parallaxMapMap;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& parallaxmap : input.getSequence("parallaxmaps"))
	{
		parallaxMapMap[this->getResourceLink(parallaxmap)] = input.getTag(parallaxmap + ".name");
	}
	return parallaxMapMap;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveDisplacementMaps() const
{
	std::unordered_map<std::string, std::string> displacementMapMap;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& displacementmap : input.getSequence("displacementmaps"))
	{
		displacementMapMap[this->getResourceLink(displacementmap)] = input.getTag(displacementmap + ".name");
	}
	return displacementMapMap;
}

unsigned int DataTranslation::retrieveAllData(std::vector<std::unique_ptr<Mesh>>& allMeshes, std::vector<std::unique_ptr<Texture>>& allTextures, std::vector<std::unique_ptr<NormalMap>>& allNormalMaps, std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps, std::vector<std::unique_ptr<DisplacementMap>>& allDisplacementMaps) const
{
	std::unordered_map<std::string, std::string> models = this->retrieveModels(), textures = this->retrieveTextures(), normalmaps = this->retrieveNormalMaps(), parallaxmaps = this->retrieveParallaxMaps(), displacementmaps = this->retrieveDisplacementMaps();
	unsigned int dataCount = 0;
	for(auto& it : models)
	{
		allMeshes.push_back(std::make_unique<Mesh>(it.first));
		dataCount++;
	}
	for(auto& it : textures)
	{
		allTextures.push_back(std::make_unique<Texture>(it.first));
		dataCount++;
	}
	for(auto& it : normalmaps)
	{
		allNormalMaps.push_back(std::make_unique<NormalMap>(it.first));
		dataCount++;
	}
	for(auto& it : parallaxmaps)
	{
		allParallaxMaps.push_back(std::make_unique<ParallaxMap>(it.first));
		dataCount++;
	}
	for(auto& it : displacementmaps)
	{
		allDisplacementMaps.push_back(std::make_unique<DisplacementMap>(it.first));
		dataCount++;
	}
	return dataCount;
}

