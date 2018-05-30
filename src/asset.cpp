#include "asset.hpp"

AssetBuffer::AssetBuffer(std::vector<std::shared_ptr<Mesh>> meshes, std::vector<std::shared_ptr<Texture>> textures, std::vector<std::shared_ptr<NormalMap>> normal_maps, std::vector<std::shared_ptr<ParallaxMap>> parallax_maps, std::vector<std::shared_ptr<DisplacementMap>> displacement_maps): meshes(meshes), textures(textures), normal_maps(normal_maps), parallax_maps(parallax_maps), displacement_maps(displacement_maps){}

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