template<typename AssetType, typename... Args>
AssetType& AssetBuffer::emplace(const std::string& asset_name, Args&&... args)
{
    if constexpr(std::is_same_v<AssetType, Mesh>)
        return this->emplace_mesh(std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, Texture>)
        return this->emplace_texture(std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, NormalMap>)
        return this->emplace_normalmap(std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, ParallaxMap>)
        return this->emplace_parallaxmap(std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, DisplacementMap>)
        return this->emplace_displacementmap(std::forward<Args>(args)...);
}

template<typename... Args>
Mesh& AssetBuffer::emplace_mesh(const std::string& asset_name, Args&&... args)
{
    std::shared_ptr<Mesh>& mesh_ptr = (*(this->meshes.insert({asset_name, std::make_shared<Mesh>(std::forward<Args>(args)...)}).first)).second;
    return *mesh_ptr.get();
}

template<typename... Args>
Texture& AssetBuffer::emplace_texture(const std::string& asset_name, Args&&... args)
{
    std::shared_ptr<Texture>& texture_ptr = (*(this->textures.insert({asset_name, std::make_shared<Texture>(std::forward<Args>(args)...)}).first)).second;
    return *texture_ptr.get();
}

template<typename... Args>
NormalMap& AssetBuffer::emplace_normalmap(const std::string& asset_name, Args&&... args)
{
    std::shared_ptr<NormalMap>& normal_map_ptr = (*(this->normal_maps.insert({asset_name, std::make_shared<NormalMap>(std::forward<Args>(args)...)}).first)).second;
    return *normal_map_ptr.get();
}

template<typename... Args>
ParallaxMap& AssetBuffer::emplace_parallaxmap(const std::string& asset_name, Args&&... args)
{
    std::shared_ptr<ParallaxMap>& parallax_map_ptr = (*(this->parallax_maps.insert({asset_name, std::make_shared<ParallaxMap>(std::forward<Args>(args)...)}).first)).second;
    return *parallax_map_ptr.get();
}

template<typename... Args>
DisplacementMap& AssetBuffer::emplace_displacementmap(const std::string& asset_name, Args&&... args)
{
    std::shared_ptr<DisplacementMap>& displacement_map_ptr = (*(this->displacement_maps.insert({asset_name, std::make_shared<DisplacementMap>(std::forward<Args>(args)...)}).first)).second;
    return *displacement_map_ptr.get();
}

template<class AssetType>
std::shared_ptr<AssetType> AssetBuffer::find(const std::string& asset_name)
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
}