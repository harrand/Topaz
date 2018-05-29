#ifndef DATA_HPP
#define DATA_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include "MDL/mdl_file.hpp"
#include <unordered_map>
#include <memory>
#include <initializer_list>

struct AssetBuffer
{
    AssetBuffer(std::vector<std::shared_ptr<Mesh>> meshes, std::vector<std::shared_ptr<Texture>> textures, std::vector<std::shared_ptr<NormalMap>> normal_maps = {}, std::vector<std::shared_ptr<ParallaxMap>> parallax_maps = {}, std::vector<std::shared_ptr<DisplacementMap>> displacement_maps = {});
	/// Container of Mesh assets.
	std::vector<std::shared_ptr<Mesh>> meshes;
	/// Container of Texture assets.
	std::vector<std::shared_ptr<Texture>> textures;
	/// Container of NormalMap assets.
	std::vector<std::shared_ptr<NormalMap>> normal_maps;
	/// Container of ParallaxMap assets.
	std::vector<std::shared_ptr<ParallaxMap>> parallax_maps;
	/// Container of DisplacementMap assets.
	std::vector<std::shared_ptr<DisplacementMap>> displacement_maps;
};

struct Asset
{
    Asset(std::weak_ptr<Mesh> mesh, std::weak_ptr<Texture> texture, std::weak_ptr<NormalMap> normal_map = {}, std::weak_ptr<ParallaxMap> parallax_map = {}, std::weak_ptr<DisplacementMap> displacement_map = {});
    bool valid_mesh() const;
    bool valid_texture() const;
    bool valid_normal_map() const;
    bool valid_parallax_map() const;
    bool valid_displacement_map() const;

    std::weak_ptr<Mesh> mesh;
    std::weak_ptr<Texture> texture;
    std::weak_ptr<NormalMap> normal_map;
    std::weak_ptr<ParallaxMap> parallax_map;
    std::weak_ptr<DisplacementMap> displacement_map;
};

namespace tz
{
	namespace data
	{
        template <typename T>
        bool is_uninitialized(std::weak_ptr<T> const& weak)
        {
            return !weak.owner_before(std::weak_ptr<T>{}) && !std::weak_ptr<T>{}.owner_before(weak);
        }
		//  Angelo was here 01/11/2017 <3
		constexpr char default_models_sequence_name[] = "models";
		constexpr char default_textures_sequence_name[] = "textures";
		constexpr char default_normal_maps_sequence_name[] = "normalmaps";
		constexpr char default_parallax_maps_sequence_name[] = "parallaxmaps";
		constexpr char default_displacement_maps_sequence_name[] = "displacementmaps";
		
		/**
		* Essentially an MDL Asset Manager. This is how Topaz handles MDL data files which hold assets such as textures and models.
		*/
		class Manager
		{
		public:
			/**
			 * Construct a data-manager from an existing MDL file.
			 * The MDLFile should really be a ResourcesFile.
			 * @param datafilename - Path to the required MDL file
			 */
			Manager(std::string datafilename);
			
			/**
				To help explain what a resource_link and a resource_name is, consider the following example:
				==example.mdl==
				lava_texture.path: "./textures/lava.png"
				==eof==
				The resource_name is the tag-name until the first '.' ("lava_texture"), and the resource_link is the value of the tag resource_name.path ("./textures/lava.png").
			 * @param resource_name - Name of the resource, as specified above
			 * @return - The link to the resource
			 */
			std::string resource_link(const std::string& resource_name) const;
			/**
			 * Retrieve the name of the resource that has the specified link.
			 * @param resource_link - Link to equate to the resource-name
			 * @return - Resource-name of the resource that conains this link. If none exists, returns mdl::default_string
			 */
			std::string resource_name(const std::string& resource_link) const;
			/**
			 * Retrieve the model resources.
			 * @param sequence_name - Name of the sequence containing all the model resources
			 * @return - Map of resources as: [link => name]
			 */
			std::unordered_map<std::string, std::string> retrieve_models(const char* sequence_name = tz::data::default_models_sequence_name) const;
			/**
			 * Retrieve the texture resources.
			 * @param sequence_name - Name of the sequence containing all the texture resources
			 * @return - Map of resources as: [link => name]
			 */
			std::unordered_map<std::string, std::string> retrieve_textures(const char* sequence_name = tz::data::default_textures_sequence_name) const;
			/**
			 * Retrieve the normal-map resources.
			 * @param sequence_name - Name of the sequence containing all the normal-map resources
			 * @return - Map of resources as: [link => name]
			 */
			std::unordered_map<std::string, std::string> retrieve_normal_maps(const char* sequence_name = tz::data::default_normal_maps_sequence_name) const;
			/**
			 * Retrieve the parallax-map resources.
			 * @param sequence_name - Name of the sequence containing all the parallax-map resources
			 * @return - Map of resources as: [link => name]
			 */
			std::unordered_map<std::string, std::string> retrieve_parallax_maps(const char* sequence_name = tz::data::default_parallax_maps_sequence_name) const;
			/**
			 * Retrieve the displacement-map resources.
			 * @param sequence_name - Name of the sequence containing all the displacement-map resources
			 * @return - Map of resources as: [link => name]
			 */
			std::unordered_map<std::string, std::string> retrieve_displacement_maps(const char* sequence_name = tz::data::default_displacement_maps_sequence_name) const;
			/**
			 * Fill all the provided containers with all resources in the file, sorted.
			 * @param all_meshes - Container to be filled with all the mesh resources
			 * @param all_textures - Container to be filled with all the texture resources
			 * @param all_normalmaps - Container to be filled with all the normal-map resources
			 * @param all_parallaxmaps - Container to be filled with all the parallax-map resources
			 * @param all_displacementmaps - Container to be filled with all the displacement-map resources
			 * @return
			 */
			unsigned int retrieve_all_data(std::vector<std::unique_ptr<Mesh>>& all_meshes, std::vector<std::unique_ptr<Texture>>& all_textures, std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps) const;
		private:
			/// Underlying MDLFile object.
			const MDLFile data_file;
		};
	}
}

#endif