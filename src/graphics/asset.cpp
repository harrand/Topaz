#include "asset.hpp"

AssetBuffer::AssetBuffer(std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes, std::unordered_map<std::string, std::shared_ptr<Texture>> textures, std::unordered_map<std::string, std::shared_ptr<NormalMap>> normal_maps, std::unordered_map<std::string, std::shared_ptr<ParallaxMap>> parallax_maps, std::unordered_map<std::string, std::shared_ptr<DisplacementMap>> displacement_maps): meshes(meshes), textures(textures), normal_maps(normal_maps), parallax_maps(parallax_maps), displacement_maps(displacement_maps){}

std::shared_ptr<Mesh> AssetBuffer::find_mesh(const std::string& mesh_name)
{
    try
    {
        return this->meshes.at(mesh_name);
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

std::shared_ptr<Texture> AssetBuffer::find_texture(const std::string& texture_name)
{
    try
    {
        return this->textures.at(texture_name);
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

std::shared_ptr<NormalMap> AssetBuffer::find_normal_map(const std::string& normal_map_name)
{
    try
    {
        return this->normal_maps.at(normal_map_name);
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

std::shared_ptr<ParallaxMap> AssetBuffer::find_parallax_map(const std::string& parallax_map_name)
{
    try
    {
        return this->parallax_maps.at(parallax_map_name);
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

std::shared_ptr<DisplacementMap> AssetBuffer::find_displacement_map(const std::string& displacement_map_name)
{
    try
    {
        return this->displacement_maps.at(displacement_map_name);
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

Asset::Asset(std::weak_ptr<Mesh> mesh, std::weak_ptr<Texture> texture, std::weak_ptr<NormalMap> normal_map, std::weak_ptr<ParallaxMap> parallax_map, std::weak_ptr<DisplacementMap> displacement_map): mesh(mesh), texture(texture), normal_map(normal_map), parallax_map(parallax_map), displacement_map(displacement_map){}

bool Asset::valid_mesh() const
{
	return !tz::data::is_uninitialized(this->mesh) && !this->mesh.expired();
}

bool Asset::valid_texture() const
{
	return !tz::data::is_uninitialized(this->texture) && !this->texture.expired();
}

bool Asset::valid_normal_map() const
{
	return !tz::data::is_uninitialized(this->normal_map) && !this->normal_map.expired();
}

bool Asset::valid_parallax_map() const
{
	return !tz::data::is_uninitialized(this->parallax_map) && !this->parallax_map.expired();
}

bool Asset::valid_displacement_map() const
{
	return !tz::data::is_uninitialized(this->displacement_map) && !this->displacement_map.expired();
}