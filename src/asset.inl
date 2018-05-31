template<typename AssetType, typename... Args>
AssetType& AssetBuffer::emplace(Args&&... args)
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
Mesh& AssetBuffer::emplace_mesh(Args&&... args)
{
    std::shared_ptr<Mesh>& mesh_ptr = this->meshes.emplace_back(std::make_shared<Mesh>(std::forward<Args>(args)...));
    return *mesh_ptr.get();
}

template<typename... Args>
Texture& AssetBuffer::emplace_texture(Args&&... args)
{
    std::shared_ptr<Texture>& texture_ptr = this->textures.emplace_back(std::make_shared<Texture>(std::forward<Args>(args)...));
    return *texture_ptr.get();
}

template<typename... Args>
NormalMap& AssetBuffer::emplace_normalmap(Args&&... args)
{
    std::shared_ptr<NormalMap>& normal_map_ptr = this->normal_maps.emplace_back(std::make_shared<NormalMap>(std::forward<Args>(args)...));
    return *normal_map_ptr.get();
}

template<typename... Args>
ParallaxMap& AssetBuffer::emplace_parallaxmap(Args&&... args)
{
    std::shared_ptr<ParallaxMap>& parallax_map_ptr = this->parallax_maps.emplace_back(std::make_shared<ParallaxMap>(std::forward<Args>(args)...));
    return *parallax_map_ptr.get();
}

template<typename... Args>
DisplacementMap& AssetBuffer::emplace_displacementmap(Args&&... args)
{
    std::shared_ptr<DisplacementMap>& displacement_map_ptr = this->displacement_maps.emplace_back(std::make_shared<DisplacementMap>(std::forward<Args>(args)...));
    return *displacement_map_ptr.get();
}