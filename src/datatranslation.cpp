#include "datatranslation.hpp"
#include "utility.hpp"

DataTranslation::DataTranslation(std::string datafilename): datafilename(std::move(datafilename)){}

std::string DataTranslation::getResourceLink(const std::string& resource_name) const
{
	return MDLF(RawFile(this->datafilename)).getTag(resource_name + ".path");
}

std::string DataTranslation::getResourceName(const std::string& resource_link) const
{
	const RawFile input(this->datafilename);
	std::vector<std::string> lines = input.getLines();
	for(std::string& line : lines)
	{
		std::vector<std::string> lineSplit = stringutility::splitString(line, ':');
		if(lineSplit.size() != 0)
		{
			std::string tagname = lineSplit.at(0);
			if(MDLF(input).getTag(tagname) == resource_link)
			{
				std::string undesired_suffix = ".path";
				tagname.erase(tagname.find(undesired_suffix), undesired_suffix.length());
				return tagname;
			}
		}
	}
	return "0";
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveModels() const
{
	std::unordered_map<std::string, std::string> model_map;
	MDLF input(RawFile(this->datafilename));
	for(std::string& model : input.getSequence("models"))
	{
		model_map[this->getResourceLink(model)] = input.getTag(model + ".name");
	}
	return model_map;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveTextures() const
{
	std::unordered_map<std::string, std::string> texture_map;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& texture : input.getSequence("textures"))
	{
		texture_map[this->getResourceLink(texture)] = input.getTag(texture + ".name");
	}
	return texture_map;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveNormalMaps() const
{
	std::unordered_map<std::string, std::string> normalmap_map;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& normalmap : input.getSequence("normalmaps"))
	{
		normalmap_map[this->getResourceLink(normalmap)] = input.getTag(normalmap + ".name");
	}
	return normalmap_map;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveParallaxMaps() const
{
	std::unordered_map<std::string, std::string> parallaxmap_map;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& parallaxmap : input.getSequence("parallaxmaps"))
	{
		parallaxmap_map[this->getResourceLink(parallaxmap)] = input.getTag(parallaxmap + ".name");
	}
	return parallaxmap_map;
}

std::unordered_map<std::string, std::string> DataTranslation::retrieveDisplacementMaps() const
{
	std::unordered_map<std::string, std::string> displacementmap_map;
	MDLF input(RawFile(this->datafilename));
	for(const std::string& displacementmap : input.getSequence("displacementmaps"))
	{
		displacementmap_map[this->getResourceLink(displacementmap)] = input.getTag(displacementmap + ".name");
	}
	return displacementmap_map;
}

unsigned int DataTranslation::retrieveAllData(std::vector<std::unique_ptr<Mesh>>& all_meshes, std::vector<std::unique_ptr<Texture>>& all_textures, std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps) const
{
	std::unordered_map<std::string, std::string> models = this->retrieveModels(), textures = this->retrieveTextures(), normalmaps = this->retrieveNormalMaps(), parallaxmaps = this->retrieveParallaxMaps(), displacementmaps = this->retrieveDisplacementMaps();
	unsigned int data_count = 0;
	for(auto& it : models)
	{
		all_meshes.push_back(std::make_unique<Mesh>(it.first));
		data_count++;
	}
	for(auto& it : textures)
	{
		all_textures.push_back(std::make_unique<Texture>(it.first));
		data_count++;
	}
	for(auto& it : normalmaps)
	{
		all_normalmaps.push_back(std::make_unique<NormalMap>(it.first));
		data_count++;
	}
	for(auto& it : parallaxmaps)
	{
		all_parallaxmaps.push_back(std::make_unique<ParallaxMap>(it.first));
		data_count++;
	}
	for(auto& it : displacementmaps)
	{
		all_displacementmaps.push_back(std::make_unique<DisplacementMap>(it.first));
		data_count++;
	}
	return data_count;
}

