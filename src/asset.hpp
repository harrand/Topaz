#ifndef DATA_HPP
#define DATA_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include <unordered_map>
#include <memory>
#include <initializer_list>

struct AssetBuffer
{
    AssetBuffer(std::vector<std::shared_ptr<Mesh>> meshes, std::vector<std::shared_ptr<Texture>> textures, std::vector<std::shared_ptr<NormalMap>> normal_maps = {}, std::vector<std::shared_ptr<ParallaxMap>> parallax_maps = {}, std::vector<std::shared_ptr<DisplacementMap>> displacement_maps = {});
	template<typename AssetType, typename... Args>
	AssetType& emplace(Args&&... args);
	template<typename... Args>
	Mesh& emplace_mesh(Args&&... args);
	template<typename... Args>
	Texture& emplace_texture(Args&&... args);
	template<typename... Args>
	NormalMap& emplace_normalmap(Args&&... args);
	template<typename... Args>
	ParallaxMap& emplace_parallaxmap(Args&&... args);
	template<typename... Args>
	DisplacementMap& emplace_displacementmap(Args&&... args);
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
	}
}

#include "asset.inl"

#endif