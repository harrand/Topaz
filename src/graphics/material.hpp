#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include "texture.hpp"

/**
 * Non-owning container for a tuple of Texture, NormalMap, ParallaxMap and DisplacementMap.
 * Objects contain one of these for simplicity's sake.
 */
class Material
{
public:
    /**
     * Construct a Material using existing properties. NormalMap, ParallaxMap and DisplacementMap are all defaulted to not exist.
     * @param texture - Pointer to existing Texture to use. If nullptr is specified, the default texture is used
     * @param normal_map - Pointer to the existing NormalMap to use. If nullptr is specified, the default normal-map is used
     * @param parallax_map - Pointer to the existing ParallaxMap to use. If nullptr is specified, the default parallax-map is used
     * @param displacement_map - Pointer to the exsiting DisplacementMap to use. If nullptr is specified, the default displacement-map is used
     */
    Material(Texture* texture, NormalMap* normal_map = nullptr, ParallaxMap* parallax_map = nullptr, DisplacementMap* displacement_map = nullptr);
    /**
     * Query whether the Material has a valid texture (will not accept the implicit default-texture)
     * @return - True if the Texture component is not null. False otherwise
     */
    bool has_texture() const;
    /**
     * Get the underlying Texture component.
     * @return - Pointer to the existing Texture in-use
     */
    const Texture* get_texture() const;
    /**
     * Query whether the Material has a valid normal-map (will not accept the implicit default-normal-map)
     * @return - True if the NormalMap component is not null. False otherwise
     */
    bool has_normal_map() const;
    /**
     * Get the underlying NormalMap component.
     * @return - Pointer to the existing NormalMap in-use
     */
    const NormalMap* get_normal_map() const;
    /**
     * Query whether the Material has a valid parallax-map (will not accept the implicit default-parallax-map)
     * @return - True if the ParallaxMap component is not null. False otherwise
     */
    bool has_parallax_map() const;
    /**
     * Get the underlying ParallaxMap component.
     * @return - Pointer to the existing ParallaxMap in-use
     */
    const ParallaxMap* get_parallax_map() const;
    /**
     * Query whether the Material has a valid displacement-map (will not accept the implicit default-displacement-map)
     * @return - True if the DisplacementMap component is not null. False otherwise
     */
    bool has_displacement_map() const;
    /**
     * Get the underlying DisplacementMap component.
     * @return - Pointer to the existing DisplacementMap in-use
     */
    const DisplacementMap* get_displacement_map() const;
    /**
     * Assign this Material to use another Texture.
     * @param texture - The Texture for the Material to use.
     */
    void set_texture(Texture* texture);
    /**
     * Assign this Material to use another NormalMap.
     * @param normal_map - The NormalMap for the Material to use.
     */
    void set_normal_map(NormalMap* normal_map);
    /**
     * Assign this Material to use another ParallaxMap.
     * @param parallax_map - The NormalMap for the Material to use.
     */
    void set_parallax_map(ParallaxMap* parallax_map);
    /**
     * Assign this Material to use another DisplacementMap.
     * @param displacement_map - The DisplacementMap for the Material to use.
     */
    void set_displacement_map(DisplacementMap* displacement_map);
    /**
     * Bind the Texture, NormalMap, ParallaxMap and DisplacementMap components to the specified Shader.
     * @param shader - The Shader to bind this Material to
     */
    virtual void bind(Shader& shader) const;

    /**
     * Equate this Material with another.
     * @param rhs - The other Material to equate with
     * @return - True if the Materials are equal. Otherwise false
     */
    bool operator==(const Material& rhs) const;
private:
    /// Texture component.
    Texture* texture;
    /// NormalMap component.
    NormalMap* normal_map;
    /// ParallaxMap component.
    ParallaxMap* parallax_map;
    /// DisplacementMap component.
    DisplacementMap* displacement_map;
};

#endif //MATERIAL_HPP