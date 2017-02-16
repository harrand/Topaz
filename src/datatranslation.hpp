#ifndef DATATRANSLATION_HPP
#define DATATRANSLATION_HPP
#include "utility.hpp"
#include <map>
#define RES_POINT std::string("../../../res/runtime")

class DataTranslation
{
public:
	DataTranslation(std::string datafilename);
	std::string getMeshLink(std::string meshName);
	std::string getTextureLink(std::string textureName);
	std::string getMeshName(std::string meshLink);
	std::string getTextureName(std::string textureLink);
	
	std::map<std::string, std::string> retrieveModels();
	std::map<std::string, std::string> retrieveTextures();
private:
	std::string datafilename;
};

#endif