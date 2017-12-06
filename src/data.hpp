#ifndef DATA_HPP
#define DATA_HPP
#include "mesh.hpp"
#include "texture.hpp"
#include "MDL/mdl.hpp"
#include <unordered_map>
#include <memory>

namespace tz
{
	namespace data
	{
		//  Angelo was here 01/11/2017 <3
		constexpr char default_models_sequence_name[] = "models";
		constexpr char default_textures_sequence_name[] = "textures";
		constexpr char default_normal_maps_sequence_name[] = "normalmaps";
		constexpr char default_parallax_maps_sequence_name[] = "parallaxmaps";
		constexpr char default_displacement_maps_sequence_name[] = "displacementmaps";
		
		/*
			Essentially an MDL Asset Manager. This is how Topaz handles MDL data files which hold assets such as textures and models.
		*/
		class Manager
		{
		public:
			Manager(std::string datafilename);
			Manager(const Manager& copy) = default;
			Manager(Manager&& move) = default;
			Manager& operator=(const Manager& rhs) = default;
			~Manager() = default;
			
			/*
				To help explain what a resource_link and a resource_name is, consider the following example:
				==example.mdl==
				lava_texture.path: "./textures/lava.png"
				==eof==
				The resource_name is the tag-name until the first '.' ("lava_texture"), and the resource_link is the value of the tag resource_name.path ("./textures/lava.png").
			*/
			std::string resource_link(const std::string& resource_name) const;
			std::string resource_name(const std::string& resource_link) const;
			std::unordered_map<std::string, std::string> retrieve_models(const char* sequence_name = tz::data::default_models_sequence_name) const;
			std::unordered_map<std::string, std::string> retrieve_textures(const char* sequence_name = tz::data::default_textures_sequence_name) const;
			std::unordered_map<std::string, std::string> retrieve_normal_maps(const char* sequence_name = tz::data::default_normal_maps_sequence_name) const;
			std::unordered_map<std::string, std::string> retrieve_parallax_maps(const char* sequence_name = tz::data::default_parallax_maps_sequence_name) const;
			std::unordered_map<std::string, std::string> retrieve_displacement_maps(const char* sequence_name = tz::data::default_displacement_maps_sequence_name) const;
			unsigned int retrieve_all_data(std::vector<std::unique_ptr<Mesh>>& all_meshes, std::vector<std::unique_ptr<Texture>>& all_textures, std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps) const;
		private:
			const std::string datafilename;
			const MDLF data_file;
		};
	}
}

#endif