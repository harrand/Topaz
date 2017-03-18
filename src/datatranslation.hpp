#ifndef DATATRANSLATION_HPP
#define DATATRANSLATION_HPP
#include "utility.hpp"
#include "mdl.hpp"
#include <map>
#define RES_POINT std::string("../../../res/runtime")

class DataTranslation
{
public:
	DataTranslation(std::string datafilename);

	std::string getResourceLink(std::string resourceName);
	std::string getResourceName(std::string resourceLink);
	
	std::map<std::string, std::string> retrieveModels();
	std::map<std::string, std::string> retrieveTextures();
	std::map<std::string, std::string> retrieveNormalMaps();
private:
	std::string datafilename;
};

#endif