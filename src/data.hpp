#ifndef DATATRANSLATION_HPP
#define DATATRANSLATION_HPP
#include "mesh.hpp"
#include "texture.hpp"
#include "mdl.hpp"
#include <unordered_map>
#include <memory>

namespace tz
{
	namespace data
	{
		class Manager
		{
		public:
			Manager(std::string datafilename);
			Manager(const Manager& copy) = default;
			Manager(Manager&& move) = default;
			Manager& operator=(const Manager& rhs) = default;
			~Manager() = default;
		
			std::string getResourceLink(const std::string& resourceName) const;
			std::string getResourceName(const std::string& resourceLink) const;
			std::unordered_map<std::string, std::string> retrieveModels() const;
			std::unordered_map<std::string, std::string> retrieveTextures() const;
			std::unordered_map<std::string, std::string> retrieveNormalMaps() const;
			std::unordered_map<std::string, std::string> retrieveParallaxMaps() const;
			std::unordered_map<std::string, std::string> retrieveDisplacementMaps() const;
			unsigned int retrieveAllData(std::vector<std::unique_ptr<Mesh>>& all_meshes, std::vector<std::unique_ptr<Texture>>& all_textures, std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps) const;
		private:
			const std::string datafilename;
			const MDLF data_file;
		};
	}
}

#endif