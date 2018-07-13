#include "graphics/asset.hpp"

AssetBuffer::AssetBuffer(std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes, std::unordered_map<std::string, std::unique_ptr<Texture>> textures, std::unordered_map<std::string, std::unique_ptr<NormalMap>> normal_maps, std::unordered_map<std::string, std::unique_ptr<ParallaxMap>> parallax_maps, std::unordered_map<std::string, std::unique_ptr<DisplacementMap>> displacement_maps): meshes(std::move(meshes)), textures(std::move(textures)), normal_maps(std::move(normal_maps)), parallax_maps(std::move(parallax_maps)), displacement_maps(std::move(displacement_maps)){}

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