#include "graphics/asset.hpp"

AssetBuffer::AssetBuffer(std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes, std::unordered_map<std::string, std::unique_ptr<Texture>> textures, std::unordered_map<std::string, std::unique_ptr<NormalMap>> normal_maps, std::unordered_map<std::string, std::unique_ptr<ParallaxMap>> parallax_maps, std::unordered_map<std::string, std::unique_ptr<DisplacementMap>> displacement_maps, std::unordered_map<std::string, std::unique_ptr<AnimatedTexture>> animated_textures): meshes(std::move(meshes)), textures(std::move(textures)), normal_maps(std::move(normal_maps)), parallax_maps(std::move(parallax_maps)), displacement_maps(std::move(displacement_maps)), animated_textures(std::move(animated_textures)){}

bool AssetBuffer::sink_mesh(const std::string& asset_name, std::unique_ptr<Mesh> sunken_mesh)
{
    if(this->meshes.find(asset_name) != this->meshes.end())
        return false;
    this->meshes[asset_name] = std::move(sunken_mesh);
    return true;
}

bool AssetBuffer::sink_texture(const std::string& asset_name, std::unique_ptr<Texture> sunken_texture)
{
    if(this->textures.find(asset_name) != this->textures.end())
        return false;
    this->textures[asset_name] = std::move(sunken_texture);
    return true;
}

bool AssetBuffer::sink_normalmap(const std::string& asset_name, std::unique_ptr<NormalMap> sunken_normalmap)
{
    if(this->normal_maps.find(asset_name) != this->normal_maps.end())
        return false;
    this->normal_maps[asset_name] = std::move(sunken_normalmap);
    return true;
}

bool AssetBuffer::sink_parallaxmap(const std::string& asset_name, std::unique_ptr<ParallaxMap> sunken_parallaxmap)
{
    if(this->parallax_maps.find(asset_name) != this->parallax_maps.end())
        return false;
    this->parallax_maps[asset_name] = std::move(sunken_parallaxmap);
    return true;
}

bool AssetBuffer::sink_displacementmap(const std::string& asset_name, std::unique_ptr<DisplacementMap> sunken_displacementmap)
{
    if(this->displacement_maps.find(asset_name) != this->displacement_maps.end())
        return false;
    this->displacement_maps[asset_name] = std::move(sunken_displacementmap);
    return true;
}

AnimatedTexture& AssetBuffer::emplace_animated_texture(const std::string& animation_name, PolyFrameTexture::FrameMap frames, unsigned int fps)
{
    auto pair_return = this->animated_textures.emplace(animation_name, std::make_unique<AnimatedTexture>(frames, fps));
    return *(*pair_return.first).second;
}

Mesh* AssetBuffer::find_mesh(const std::string& mesh_name)
{
    try
    {
        return this->meshes.at(mesh_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

Texture* AssetBuffer::find_texture(const std::string& texture_name)
{
    try
    {
        return this->textures.at(texture_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

NormalMap* AssetBuffer::find_normal_map(const std::string& normal_map_name)
{
    try
    {
        return this->normal_maps.at(normal_map_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

ParallaxMap* AssetBuffer::find_parallax_map(const std::string& parallax_map_name)
{
    try
    {
        return this->parallax_maps.at(parallax_map_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

DisplacementMap* AssetBuffer::find_displacement_map(const std::string& displacement_map_name)
{
    try
    {
        return this->displacement_maps.at(displacement_map_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

AnimatedTexture* AssetBuffer::find_animated_texture(const std::string& animation_name)
{
    try
    {
        return this->animated_textures.at(animation_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

std::unique_ptr<Mesh> AssetBuffer::take_mesh(const std::string& mesh_name)
{
    if(this->meshes.find(mesh_name) == this->meshes.end())
        return nullptr;
    std::unique_ptr<Mesh> mesh = std::move(this->meshes[mesh_name]);
    this->meshes.erase(mesh_name);
    return mesh;
}

std::unique_ptr<Texture> AssetBuffer::take_texture(const std::string& texture_name)
{
    if(this->textures.find(texture_name) == this->textures.end())
        return nullptr;
    std::unique_ptr<Texture> texture = std::move(this->textures[texture_name]);
    this->textures.erase(texture_name);
    return texture;
}

std::unique_ptr<NormalMap> AssetBuffer::take_normalmap(const std::string& normalmap_name)
{
    if(this->normal_maps.find(normalmap_name) == this->normal_maps.end())
        return nullptr;
    std::unique_ptr<NormalMap> normalmap = std::move(this->normal_maps[normalmap_name]);
    this->normal_maps.erase(normalmap_name);
    return normalmap;
}

std::unique_ptr<ParallaxMap> AssetBuffer::take_parallaxmap(const std::string& parallaxmap_name)
{
    if(this->parallax_maps.find(parallaxmap_name) == this->parallax_maps.end())
        return nullptr;
    std::unique_ptr<ParallaxMap> parallaxmap = std::move(this->parallax_maps[parallaxmap_name]);
    this->parallax_maps.erase(parallaxmap_name);
    return parallaxmap;
}

std::unique_ptr<DisplacementMap> AssetBuffer::take_displacementmap(const std::string& displacementmap_name)
{
    if(this->displacement_maps.find(displacementmap_name) == this->displacement_maps.end())
        return nullptr;
    std::unique_ptr<DisplacementMap> displacementmap = std::move(this->displacement_maps[displacementmap_name]);
    this->displacement_maps.erase(displacementmap_name);
    return displacementmap;
}

Asset::Asset(Mesh* mesh, Texture* texture, NormalMap* normal_map, ParallaxMap* parallax_map, DisplacementMap* displacement_map): mesh(mesh), texture(texture), normal_map(normal_map), parallax_map(parallax_map), displacement_map(displacement_map){}

bool Asset::valid_mesh() const
{
	return this->mesh != nullptr;
}

bool Asset::valid_texture() const
{
	return this->texture != nullptr;
}

bool Asset::valid_normal_map() const
{
	return this->normal_map != nullptr;
}

bool Asset::valid_parallax_map() const
{
	return this->parallax_map != nullptr;
}

bool Asset::valid_displacement_map() const
{
	return this->displacement_map != nullptr;
}

bool Asset::operator==(const Asset &rhs) const
{
    return this->mesh == rhs.mesh && this->texture == rhs.texture && this->normal_map == rhs.normal_map && this->parallax_map == rhs.parallax_map && this->displacement_map == rhs.displacement_map;
}