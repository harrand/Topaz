#ifndef DATA_HPP
#define DATA_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include <unordered_map>
#include <memory>
#include <initializer_list>

struct AssetBuffer
{
    AssetBuffer(std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes, std::unordered_map<std::string, std::shared_ptr<Texture>> textures, std::unordered_map<std::string, std::shared_ptr<NormalMap>> normal_maps = {}, std::unordered_map<std::string, std::shared_ptr<ParallaxMap>> parallax_maps = {}, std::unordered_map<std::string, std::shared_ptr<DisplacementMap>> displacement_maps = {});
	template<typename AssetType, typename... Args>
	AssetType& emplace(const std::string& asset_name, Args&&... args);
	template<typename... Args>
	Mesh& emplace_mesh(const std::string& asset_name, Args&&... args);
	template<typename... Args>
	Texture& emplace_texture(const std::string& asset_name, Args&&... args);
	template<typename... Args>
	NormalMap& emplace_normalmap(const std::string& asset_name, Args&&... args);
	template<typename... Args>
	ParallaxMap& emplace_parallaxmap(const std::string& asset_name, Args&&... args);
	template<typename... Args>
	DisplacementMap& emplace_displacementmap(const std::string& asset_name, Args&&... args);
    template<class AssetType>
    std::shared_ptr<AssetType> find(const std::string& asset_name);
    std::shared_ptr<Mesh> find_mesh(const std::string& mesh_name);
    std::shared_ptr<Texture> find_texture(const std::string& texture_name);
    std::shared_ptr<NormalMap> find_normal_map(const std::string& normal_map_name);
    std::shared_ptr<ParallaxMap> find_parallax_map(const std::string& parallax_map_name);
    std::shared_ptr<DisplacementMap> find_displacement_map(const std::string& displacement_map_name);
private:
    /// Container of Mesh assets.
	std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
	/// Container of Texture assets.
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	/// Container of NormalMap assets.
	std::unordered_map<std::string, std::shared_ptr<NormalMap>> normal_maps;
	/// Container of ParallaxMap assets.
	std::unordered_map<std::string, std::shared_ptr<ParallaxMap>> parallax_maps;
	/// Container of DisplacementMap assets.
	std::unordered_map<std::string, std::shared_ptr<DisplacementMap>> displacement_maps;
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
	}
}

#include "asset.inl"

#endif