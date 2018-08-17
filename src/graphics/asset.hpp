#ifndef DATA_HPP
#define DATA_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include <unordered_map>
#include <memory>
#include <initializer_list>

struct AssetBuffer
{
    AssetBuffer(std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes = {}, std::unordered_map<std::string, std::unique_ptr<Texture>> textures = {}, std::unordered_map<std::string, std::unique_ptr<NormalMap>> normal_maps = {}, std::unordered_map<std::string, std::unique_ptr<ParallaxMap>> parallax_maps = {}, std::unordered_map<std::string, std::unique_ptr<DisplacementMap>> displacement_maps = {});
    /// AssetBuffers are non-copyable.
    AssetBuffer(const AssetBuffer& copy) = delete;
    /// AssetBuffers are non-copyable.
    AssetBuffer& operator=(const AssetBuffer& rhs) = delete;
    bool sink_mesh(const std::string& asset_name, std::unique_ptr<Mesh> sunken_mesh);
    bool sink_texture(const std::string& asset_name, std::unique_ptr<Texture> sunken_texture);
    bool sink_normalmap(const std::string& asset_name, std::unique_ptr<NormalMap> sunken_normalmap);
    bool sink_parallaxmap(const std::string& asset_name, std::unique_ptr<ParallaxMap> sunken_parallaxmap);
    bool sink_displacementmap(const std::string& asset_name, std::unique_ptr<DisplacementMap> sunken_displacementmap);
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
    AssetType* find(const std::string& asset_name);
    Mesh* find_mesh(const std::string& mesh_name);
    Texture* find_texture(const std::string& texture_name);
    NormalMap* find_normal_map(const std::string& normal_map_name);
    ParallaxMap* find_parallax_map(const std::string& parallax_map_name);
    DisplacementMap* find_displacement_map(const std::string& displacement_map_name);
    std::unique_ptr<Mesh> take_mesh(const std::string& mesh_name);
    std::unique_ptr<Texture> take_texture(const std::string& texture_name);
    std::unique_ptr<NormalMap> take_normalmap(const std::string& normalmap_name);
    std::unique_ptr<ParallaxMap> take_parallaxmap(const std::string& parallaxmap_name);
    std::unique_ptr<DisplacementMap> take_displacementmap(const std::string& displacementmap_name);
private:
    /// Container of Mesh assets.
	std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
	/// Container of Texture assets.
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
	/// Container of NormalMap assets.
	std::unordered_map<std::string, std::unique_ptr<NormalMap>> normal_maps;
	/// Container of ParallaxMap assets.
	std::unordered_map<std::string, std::unique_ptr<ParallaxMap>> parallax_maps;
	/// Container of DisplacementMap assets.
	std::unordered_map<std::string, std::unique_ptr<DisplacementMap>> displacement_maps;
};

struct Asset
{
    Asset(Mesh* mesh, Texture* texture, NormalMap* normal_map = nullptr, ParallaxMap* parallax_map = nullptr, DisplacementMap* displacement_map = nullptr);
    bool valid_mesh() const;
    bool valid_texture() const;
    bool valid_normal_map() const;
    bool valid_parallax_map() const;
    bool valid_displacement_map() const;
    bool operator==(const Asset& rhs) const;

    Mesh* mesh;
    Texture* texture;
    NormalMap* normal_map;
    ParallaxMap* parallax_map;
    DisplacementMap* displacement_map;
};

#include "asset.inl"

#endif