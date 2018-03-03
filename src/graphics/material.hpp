#ifndef TOPAZ_MATERIAL_HPP
#define TOPAZ_MATERIAL_HPP
#include "texture.hpp"

class Material
{
public:
    Material(Texture* texture, NormalMap* normal_map, ParallaxMap* parallax_map, DisplacementMap* displacement_map);
    bool has_texture() const;
    const Texture* get_texture() const;
    bool has_normal_map() const;
    const NormalMap* get_normal_map() const;
    bool has_parallax_map() const;
    const ParallaxMap* get_parallax_map() const;
    bool has_displacement_map() const;
    const DisplacementMap* get_displacement_map() const;
    void set_texture(Texture* texture);
    void set_normal_map(NormalMap* normal_map);
    void set_parallax_map(ParallaxMap* parallax_map);
    void set_displacement_map(DisplacementMap* displacement_map);
    virtual void bind(Shader& shader) const;

    bool operator==(const Material& rhs) const;
private:
    Texture* texture;
    NormalMap* normal_map;
    ParallaxMap* parallax_map;
    DisplacementMap* displacement_map;
};


#endif //MATERIAL_HPP
