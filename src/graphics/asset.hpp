#ifndef DATA_HPP
#define DATA_HPP
#include "graphics/mesh.hpp"
#include "graphics/animated_texture.hpp"
#include <unordered_map>
#include <memory>
#include <initializer_list>

/**
 * An object used to store collections of assets.
 * Assets include:
 * 	- Meshes
 * 	- Textures
 * 	- Normal Maps
 * 	- Parallax Maps
 * 	- Displacement Maps
 * 	- Animated Textures
 */
struct AssetBuffer
{
	/**
	 * Construct an AssetBuffer from existing collections of assets.
	 * @param meshes - A map of Mesh names to their corresponding Mesh objects
	 * @param textures - A map of Texture names to their corresponding Texture objects
	 * @param normal_maps - A map of Normal Map names to their corresponding Normal Map objects
	 * @param parallax_maps - A map of Parallax Map names to their corresponding Parallax Map objects
	 * @param displacement_maps - A map of Displacement Map names to their corresponding Displacement Map objects
	 * @param animated_textures - A map of Animated-Texture names to their corresponding Animated-Texture objects
	 */
    AssetBuffer(std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes = {}, std::unordered_map<std::string, std::unique_ptr<Texture>> textures = {}, std::unordered_map<std::string, std::unique_ptr<NormalMap>> normal_maps = {}, std::unordered_map<std::string, std::unique_ptr<ParallaxMap>> parallax_maps = {}, std::unordered_map<std::string, std::unique_ptr<DisplacementMap>> displacement_maps = {}, std::unordered_map<std::string, std::unique_ptr<AnimatedTexture>> animated_textures = {});
    /// AssetBuffers are non-copyable.
    AssetBuffer(const AssetBuffer& copy) = delete;
    /// AssetBuffers are non-copyable.
    AssetBuffer& operator=(const AssetBuffer& rhs) = delete;
	/**
	 * Relinquish control of an existing Mesh asset to this AssetBuffer.
	 * After invocation, this AssetBuffer shall now be responsible for the lifetime of the Mesh asset.
	 * @param asset_name - The name of this asset
	 * @param sunken_mesh - The existing mesh asset whose lifetime should be controlled by this AssetBuffer
	 * @return - True if the AssetBuffer took control of the asset from elsewhere. False if the AssetBuffer already had control (and thus no change in control occurred)
	 */
    bool sink_mesh(const std::string& asset_name, std::unique_ptr<Mesh> sunken_mesh);
	/**
	 * Relinquish control of an existing Texture asset to this AssetBuffer.
	 * After invocation, this AssetBuffer shall now be responsible for the lifetime of the Texture asset.
	 * @param asset_name - The name of this asset
	 * @param sunken_mesh - The existing mesh asset whose lifetime should be controlled by this AssetBuffer
	 * @return - True if the AssetBuffer took control of the asset from elsewhere. False if the AssetBuffer already had control (and thus no change in control occurred)
	 */
    bool sink_texture(const std::string& asset_name, std::unique_ptr<Texture> sunken_texture);
	/**
	 * Relinquish control of an existing Normal Map asset to this AssetBuffer.
	 * After invocation, this AssetBuffer shall now be responsible for the lifetime of the Normal Map asset.
	 * @param asset_name - The name of this asset
	 * @param sunken_mesh - The existing mesh asset whose lifetime should be controlled by this AssetBuffer
	 * @return - True if the AssetBuffer took control of the asset from elsewhere. False if the AssetBuffer already had control (and thus no change in control occurred)
	 */
    bool sink_normalmap(const std::string& asset_name, std::unique_ptr<NormalMap> sunken_normalmap);
	/**
	 * Relinquish control of an existing Parallax Map asset to this AssetBuffer.
	 * After invocation, this AssetBuffer shall now be responsible for the lifetime of the Parallax Map asset.
	 * @param asset_name - The name of this asset
	 * @param sunken_mesh - The existing mesh asset whose lifetime should be controlled by this AssetBuffer
	 * @return - True if the AssetBuffer took control of the asset from elsewhere. False if the AssetBuffer already had control (and thus no change in control occurred)
	 */
    bool sink_parallaxmap(const std::string& asset_name, std::unique_ptr<ParallaxMap> sunken_parallaxmap);
	/**
	 * Relinquish control of an existing Displacement Map asset to this AssetBuffer.
	 * After invocation, this AssetBuffer shall now be responsible for the lifetime of the Displacement Map asset.
	 * @param asset_name - The name of this asset
	 * @param sunken_mesh - The existing mesh asset whose lifetime should be controlled by this AssetBuffer
	 * @return - True if the AssetBuffer took control of the asset from elsewhere. False if the AssetBuffer already had control (and thus no change in control occurred)
	 */
    bool sink_displacementmap(const std::string& asset_name, std::unique_ptr<DisplacementMap> sunken_displacementmap);
	/**
	 * Construct some asset in-place into this AssetBuffer.
	 * @tparam AssetType - The type of asset to emplace. E.g Mesh or Texture
	 * @tparam Args - Argument types for asset construction
	 * @param asset_name - Desired name of the asset
	 * @param args - Argument values for the asset construction
	 * @return - The constructed asset
	 */
	template<typename AssetType, typename... Args>
	AssetType& emplace(const std::string& asset_name, Args&&... args);
	/**
	 * Construct a Mesh in-place into this AssetBuffer
	 * Note: This is equivalent to AssetBuffer::emplace<Mesh>(asset_name, args...)
	 * @tparam Args - Argument types for asset construction
	 * @param asset_name - Desired name of the asset
	 * @param args - Argument values for the asset construction
	 * @return - The constructed Mesh.
	 */
	template<typename... Args>
	Mesh& emplace_mesh(const std::string& asset_name, Args&&... args);
	/**
	 * Construct a Texture in-place into this AssetBuffer
	 * Note: This is equivalent to AssetBuffer::emplace<Texture>(asset_name, args...)
	 * @tparam Args - Argument types for asset construction
	 * @param asset_name - Desired name of the asset
	 * @param args - Argument values for the asset construction
	 * @return - The constructed Texture.
	 */
	template<typename... Args>
	Texture& emplace_texture(const std::string& asset_name, Args&&... args);
	/**
	 * Construct a Normal Map in-place into this AssetBuffer
	 * Note: This is equivalent to AssetBuffer::emplace<NormalMap>(asset_name, args...)
	 * @tparam Args - Argument types for asset construction
	 * @param asset_name - Desired name of the asset
	 * @param args - Argument values for the asset construction
	 * @return - The constructed Normal Map.
	 */
	template<typename... Args>
	NormalMap& emplace_normalmap(const std::string& asset_name, Args&&... args);
	/**
	 * Construct a Parallax Map in-place into this AssetBuffer
	 * Note: This is equivalent to AssetBuffer::emplace<ParallaxMap>(asset_name, args...)
	 * @tparam Args - Argument types for asset construction
	 * @param asset_name - Desired name of the asset
	 * @param args - Argument values for the asset construction
	 * @return - The constructed Parallax Map.
	 */
	template<typename... Args>
	ParallaxMap& emplace_parallaxmap(const std::string& asset_name, Args&&... args);
	/**
	 * Construct a Displacement Map in-place into this AssetBuffer
	 * Note: This is equivalent to AssetBuffer::emplace<DisplacementMap>(asset_name, args...)
	 * @tparam Args - Argument types for asset construction
	 * @param asset_name - Desired name of the asset
	 * @param args - Argument values for the asset construction
	 * @return - The constructed Displacement Map.
	 */
	template<typename... Args>
	DisplacementMap& emplace_displacementmap(const std::string& asset_name, Args&&... args);
	/**
	 * Construct an Animated Texture in-place into this AssetBuffer
	 * Note: This is equivalent to AssetBuffer::emplace<AnimatedTexture>(asset_name, frames, fps)
	 * @param animation_name - Desired name of the Animated Texture
	 * @param frames - Collection of the frames constituting the Animated Texture
	 * @param fps - Desired number of frames to be displayed per second for the animation
	 * @return - The constructed Animated Texture.
	 */
	AnimatedTexture& emplace_animated_texture(const std::string& animation_name, PolyFrameTexture::FrameMap frames, unsigned int fps);
	/**
	 * Find an asset with the given name.
	 * @tparam AssetType - The type of asset to query
	 * @param asset_name - The name of the asset to retrieve
	 * @return - A pointer to the asset if it was found. If it was not found, nullptr is returned
	 */
    template<class AssetType>
    AssetType* find(const std::string& asset_name);
	/**
	 * Find a Mesh asset with the given name
	 * @param mesh_name - The name of the Mesh to query
	 * @return - A pointer to the Mesh if it was found. If it was not found, nullptr is returned.
	 */
    Mesh* find_mesh(const std::string& mesh_name);
	/**
	 * Find a Texture asset with the given name
	 * @param texture_name - The name of the Texture to query
	 * @return - A pointer to the Texture if it was found. If it was not found, nullptr is returned.
	 */
    Texture* find_texture(const std::string& texture_name);
	/**
	 * Find a Normal Map asset with the given name
	 * @param normal_map_name - The name of the Normal Map to query
	 * @return - A pointer to the Normal Map if it was found. If it was not found, nullptr is returned.
	 */
    NormalMap* find_normal_map(const std::string& normal_map_name);
	/**
	 * Find a Parallax Map asset with the given name
	 * @param parallax_map_name - The name of the Parallax Map to query
	 * @return - A pointer to the Parallax Map if it was found. If it was not found, nullptr is returned.
	 */
    ParallaxMap* find_parallax_map(const std::string& parallax_map_name);
	/**
	 * Find a Displacement Map asset with the given name
	 * @param displacement_map_name - The name of the Displacement Map to query
	 * @return - A pointer to the Displacement Map if it was found. If it was not found, nullptr is returned.
	 */
    DisplacementMap* find_displacement_map(const std::string& displacement_map_name);
	/**
	 * Find an Animated Texture asset with the given name
	 * @param animation_name - The name of the Animated Texture to query
	 * @return - A pointer to the Animated Texture if it was found. If it was not found, nullptr is returned.
	 */
    AnimatedTexture* find_animated_texture(const std::string& animation_name);
	/**
	 * Query the AssetBuffer to relinquish control of an existing Mesh. The lifetime of the asset shall no longer be controlled by the AssetBuffer.
	 * @param mesh_name - The name of the Mesh to take control of
	 * @return - The Mesh asset smart-pointer. If no such asset was found, nullptr is returned.
	 */
    std::unique_ptr<Mesh> take_mesh(const std::string& mesh_name);
	/**
	 * Query the AssetBuffer to relinquish control of an existing Texture. The lifetime of the asset shall no longer be controlled by the AssetBuffer.
	 * @param texture_name - The name of the Texture to take control of
	 * @return - The Texture asset smart-pointer. If no such asset was found, nullptr is returned.
	 */
    std::unique_ptr<Texture> take_texture(const std::string& texture_name);
	/**
	 * Query the AssetBuffer to relinquish control of an existing Normal Map. The lifetime of the asset shall no longer be controlled by the AssetBuffer.
	 * @param normalmap_name - The name of the Normal Map to take control of
	 * @return - The Normal Map asset smart-pointer. If no such asset was found, nullptr is returned.
	 */
    std::unique_ptr<NormalMap> take_normalmap(const std::string& normalmap_name);
	/**
	 * Query the AssetBuffer to relinquish control of an existing Parallax Map. The lifetime of the asset shall no longer be controlled by the AssetBuffer.
	 * @param parallaxmap_name - The name of the Parallax Map to take control of
	 * @return - The Parallax Map asset smart-pointer. If no such asset was found, nullptr is returned.
	 */
    std::unique_ptr<ParallaxMap> take_parallaxmap(const std::string& parallaxmap_name);
	/**
	 * Query the AssetBuffer to relinquish control of an existing Displacement Map. The lifetime of the asset shall no longer be controlled by the AssetBuffer.
	 * @param displacementmap_name - The name of the Displacement Map to take control of
	 * @return - The Displacement Map asset smart-pointer. If no such asset was found, nullptr is returned.
	 */
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
	/// Container of AnimatedTexture mappings.
	std::unordered_map<std::string, std::unique_ptr<AnimatedTexture>> animated_textures;
};

/**
 * A non-owning structure containing (optionally) the following:
 * - Mesh
 * - Texture
 * - Normal Map
 * - Parallax Map
 * - Displacement Map
 */
struct Asset
{
	/**
	 * Construct an Asset given existing assets.
	 * Note: Asset has no control at all over the lifetime of the asset elements it contains.
	 * @param mesh - The Mesh component
	 * @param texture - The Texture component
	 * @param normal_map - The Normal Map component
	 * @param parallax_map - The Parallax Map component
	 * @param displacement_map - The Displacement Map component
	 */
    Asset(Mesh* mesh, Texture* texture, NormalMap* normal_map = nullptr, ParallaxMap* parallax_map = nullptr, DisplacementMap* displacement_map = nullptr);
	/**
	 * Query as to whether there is a valid Mesh component.
	 * @return - True if there is a Mesh, false otherwise
	 */
    bool valid_mesh() const;
	/**
	 * Query as to whether there is a valid Texture component.
	 * @return - True if there is a Texture, false otherwise
	 */
    bool valid_texture() const;
	/**
	 * Query as to whether there is a valid Normal Map component.
	 * @return - True if there is a Normal Map, false otherwise
	 */
    bool valid_normal_map() const;
	/**
	 * Query as to whether there is a valid Parallax Map component.
	 * @return - True if there is a Parallax Map, false otherwise
	 */
    bool valid_parallax_map() const;
	/**
	 * Query as to whether there is a valid Displacement Map component.
	 * @return - True if there is a Displacement Map, false otherwise
	 */
    bool valid_displacement_map() const;
	/// Equate Assets. Returns true if the assets share the exact same asset elements.
    bool operator==(const Asset& rhs) const;

	/// The Mesh component.
    Mesh* mesh;
	/// The Texture component.
    Texture* texture;
	/// The Normal Map component.
    NormalMap* normal_map;
	/// The Parallax Map component.
    ParallaxMap* parallax_map;
	/// The Displacement Map component.
    DisplacementMap* displacement_map;
};

#include "asset.inl"

#endif