#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include "texture.hpp"

/**
 * Non-owning container for a tuple of Texture, NormalMap, ParallaxMap and DisplacementMap.
 * Objects contain one of these for simplity's sake.
 */
class Material
{
public:
    /*
     * Constructs a Material referencing the parameters. All are optional, except from Texture. If nullptr is passed for Texture, the default texture will be used.
     * If nullptr is used for any of the other parameters, default-variants shall be used.
     */
    Material(Texture* texture, NormalMap* normal_map = nullptr, ParallaxMap* parallax_map = nullptr, DisplacementMap* displacement_map = nullptr);
    /**
     * Returns true if the texture component is not null.
     * Note: This will return false if the default-texture was manually passed into the constructor, but true if nullptr was passed and the default-texture was inferred.
     */
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
