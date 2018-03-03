#include "material.hpp"

Material::Material(Texture* texture, NormalMap* normal_map, ParallaxMap* parallax_map, DisplacementMap* displacement_map): texture(texture), normal_map(normal_map), parallax_map(parallax_map), displacement_map(displacement_map){}

bool Material::has_texture() const
{
    return this->texture != nullptr;
}

const Texture* Material::get_texture() const
{
    return this->texture;
}

bool Material::has_normal_map() const
{
    return this->normal_map != nullptr;
}

const NormalMap* Material::get_normal_map() const
{
    return this->normal_map;
}

bool Material::has_parallax_map() const
{
    return this->parallax_map != nullptr;
}

const ParallaxMap* Material::get_parallax_map() const
{
    return this->parallax_map;
}

bool Material::has_displacement_map() const
{
    return this->displacement_map != nullptr;
}

const DisplacementMap* Material::get_displacement_map() const
{
    return this->displacement_map;
}

void Material::set_texture(Texture* texture)
{
    this->texture = texture;
}

void Material::set_normal_map(NormalMap* normal_map)
{
    this->normal_map = normal_map;
}

void Material::set_parallax_map(ParallaxMap* parallax_map)
{
    this->parallax_map = parallax_map;
}

void Material::set_displacement_map(DisplacementMap* displacement_map)
{
    this->displacement_map = displacement_map;
}

void Material::bind(Shader& shader) const
{
    if(this->has_texture())
        this->texture->bind(&shader, 0);
    else
    {
        tz::util::log::error("Tried to bind a Material without a Texture. Materials MUST at least have a Texture applied.");
        return;
    }
    if(this->has_normal_map())
        this->normal_map->bind(&shader, 1);
    else
        tz::graphics::texture::default_normal_map.bind(&shader, 1);
    if(this->has_parallax_map())
        this->parallax_map->bind(&shader, 2);
    else
        tz::graphics::texture::default_parallax_map.bind(&shader, 2);
    if(this->has_displacement_map())
        this->displacement_map->bind(&shader, 3);
    else
        tz::graphics::texture::default_displacement_map.bind(&shader, 3);
}

bool Material::operator==(const Material& rhs) const
{
    return this->texture == rhs.texture && this->normal_map == rhs.normal_map && this->parallax_map == rhs.parallax_map && this->displacement_map == rhs.displacement_map;
}