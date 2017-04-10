#ifndef DATATRANSLATION_HPP
#define DATATRANSLATION_HPP
#include "utility.hpp"
#include "mdl.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include <unordered_map>
#include <memory>
#define RES_POINT std::string("../../../res/runtime")

class DataTranslation
{
public:
	DataTranslation(std::string datafilename);

	std::string getResourceLink(const std::string& resourceName) const;
	std::string getResourceName(const std::string& resourceLink) const;
	
	std::unordered_map<std::string, std::string> retrieveModels() const;
	std::unordered_map<std::string, std::string> retrieveTextures() const;
	std::unordered_map<std::string, std::string> retrieveNormalMaps() const;
	std::unordered_map<std::string, std::string> retrieveParallaxMaps() const;
	
	unsigned int retrieveAllData(std::vector<std::unique_ptr<Mesh>>& allMeshes, std::vector<std::unique_ptr<Texture>>& allTextures, std::vector<std::unique_ptr<NormalMap>>& allNormalMaps, std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps) const;
private:
	const std::string datafilename;
};

#endif