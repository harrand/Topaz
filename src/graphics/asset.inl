template<typename AssetType, typename... Args>
AssetType& AssetBuffer::emplace(const std::string& asset_name, Args&&... args)
{
    if constexpr(std::is_same_v<AssetType, Mesh>)
        return this->emplace_mesh(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, Texture>)
        return this->emplace_texture(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, NormalMap>)
        return this->emplace_normalmap(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, ParallaxMap>)
        return this->emplace_parallaxmap(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, DisplacementMap>)
        return this->emplace_displacementmap(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, AnimatedTexture>)
        return this->emplace_animated_texture(asset_name, std::forward<Args>(args)...);
}

template<typename... Args>
Mesh& AssetBuffer::emplace_mesh(const std::string& asset_name, Args&&... args)
{
    std::unique_ptr<Mesh>& mesh_ptr = (*(this->meshes.insert(std::make_pair(asset_name, std::make_unique<Mesh>(std::forward<Args>(args)...))).first)).second;
    return *mesh_ptr.get();
}

template<typename... Args>
Texture& AssetBuffer::emplace_texture(const std::string& asset_name, Args&&... args)
{
    std::unique_ptr<Texture>& texture_ptr = (*(this->textures.insert(std::make_pair(asset_name, std::make_unique<Texture>(std::forward<Args>(args)...))).first)).second;
    return *texture_ptr.get();
}

template<typename... Args>
NormalMap& AssetBuffer::emplace_normalmap(const std::string& asset_name, Args&&... args)
{
    std::unique_ptr<NormalMap>& normal_map_ptr = (*(this->normal_maps.insert(std::make_pair(asset_name, std::make_unique<NormalMap>(std::forward<Args>(args)...))).first)).second;
    return *normal_map_ptr.get();
}

template<typename... Args>
ParallaxMap& AssetBuffer::emplace_parallaxmap(const std::string& asset_name, Args&&... args)
{
    std::unique_ptr<ParallaxMap>& parallax_map_ptr = (*(this->parallax_maps.insert(std::make_pair(asset_name, std::make_unique<ParallaxMap>(std::forward<Args>(args)...))).first)).second;
    return *parallax_map_ptr.get();
}

template<typename... Args>
DisplacementMap& AssetBuffer::emplace_displacementmap(const std::string& asset_name, Args&&... args)
{
    std::unique_ptr<DisplacementMap>& displacement_map_ptr = (*(this->displacement_maps.insert(std::make_pair(asset_name, std::make_unique<DisplacementMap>(std::forward<Args>(args)...))).first)).second;
    return *displacement_map_ptr.get();
}

template<class AssetType>
AssetType* AssetBuffer::find(const std::string& asset_name)
{
    if constexpr(std::is_same_v<AssetType, Mesh>)
        return this->find_mesh(asset_name);
    else if constexpr(std::is_same_v<AssetType, Texture>)
        return this->find_texture(asset_name);
    else if constexpr(std::is_same_v<AssetType, NormalMap>)
        return this->find_normal_map(asset_name);
    else if constexpr(std::is_same_v<AssetType, ParallaxMap>)
        return this->find_parallax_map(asset_name);
    else if constexpr(std::is_same_v<AssetType, DisplacementMap>)
        return this->find_displacement_map(asset_name);
    else if constexpr(std::is_same_v<AssetType, AnimatedTexture>)
        return this->find_animated_texture(asset_name);
}