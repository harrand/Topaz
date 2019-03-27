#include "utility/functional.hpp"

template<typename AssetType, typename... Args>
AssetType& AssetBuffer::emplace(const std::string& asset_name, Args&&... args)
{
    using namespace tz::utility::functional;
    if constexpr(std::is_same_v<AssetType, Mesh>)
        return this->emplace_mesh(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_base_of_v<Mesh, AssetType>)
    {
        this->meshes.emplace(asset_name, std::make_unique<AssetType>(std::forward<Args>(args)...));
        return *dynamic_cast<AssetType*>(this->meshes[asset_name].get());
    }
    else if constexpr(std::is_same_v<AssetType, Texture>)
        return this->emplace_texture(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_base_of_v<Texture, AssetType>)
    {
        this->textures.emplace(asset_name, std::make_unique<AssetType>(std::forward<Args>(args)...));
        return *dynamic_cast<AssetType*>(this->textures[asset_name].get());
    }
    else if constexpr(std::is_same_v<AssetType, AnimatedTexture>)
        return this->emplace_animated_texture(asset_name, std::forward<Args>(args)...);
    else if constexpr(std::is_same_v<AssetType, Model>)
        return this->emplace_model(asset_name, std::forward<Args>(args)...);
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
Model& AssetBuffer::emplace_model(const std::string& asset_name, Args&&... args)
{
    std::unique_ptr<Model>& model_ptr = (*(this->models.insert(std::make_pair(asset_name, std::make_unique<Model>(std::forward<Args>(args)...))).first)).second;
    return *model_ptr.get();
}

template<class AssetType>
AssetType* AssetBuffer::find(const std::string& asset_name)
{
    if constexpr(std::is_same_v<AssetType, Mesh>)
        return this->find_mesh(asset_name);
    else if constexpr(std::is_base_of_v<Mesh, AssetType>)
        return dynamic_cast<AssetType*>(this->meshes[asset_name].get());
    else if constexpr(std::is_same_v<AssetType, Texture>)
        return this->find_texture(asset_name);
    else if constexpr(std::is_base_of_v<Texture, AssetType>)
        return dynamic_cast<AssetType*>(this->textures[asset_name].get());
    else if constexpr(std::is_same_v<AssetType, AnimatedTexture>)
        return this->find_animated_texture(asset_name);
    else if constexpr(std::is_same_v<AssetType, Model>)
        return this->find_model(asset_name);
}