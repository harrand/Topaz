#ifndef DATA_HPP
#define DATA_HPP
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
			
			std::string get_resource_link(const std::string& resource_name) const;
			std::string get_resource_name(const std::string& resource_link) const;
			std::unordered_map<std::string, std::string> retrieve_models() const;
			std::unordered_map<std::string, std::string> retrieve_textures() const;
			std::unordered_map<std::string, std::string> retrieve_normal_maps() const;
			std::unordered_map<std::string, std::string> retrieve_parallax_maps() const;
			std::unordered_map<std::string, std::string> retrieve_displacement_maps() const;
			unsigned int retrieve_all_data(std::vector<std::unique_ptr<Mesh>>& all_meshes, std::vector<std::unique_ptr<Texture>>& all_textures, std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps) const;
		private:
			const std::string datafilename;
			const MDLF data_file;
		};
	}
}

#endif