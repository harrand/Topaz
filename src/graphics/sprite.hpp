#ifndef TOPAZ_SPRITE_HPP
#define TOPAZ_SPRITE_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"

class Sprite
{
public:
    Sprite(Vector2F position_screenspace, float rotation, Vector2F scale, const Texture* texture);

    virtual void render(Shader& sprite_shader, const Vector2I& viewport_dimensions) const;
    float get_rotation() const;
    void set_rotation(float radians);
    const Texture* get_texture() const;
    void set_texture(const Texture* texture);

    bool operator==(const Sprite& rhs) const;
    Vector2F position_screenspace;
protected:
    float rotation;
public:
    Vector2F scale;
protected:
    const Texture* texture;
    Mesh mesh;
};


#endif //TOPAZ_SPRITE_HPP
