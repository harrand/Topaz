#ifndef TOPAZ_SPRITE_HPP
#define TOPAZ_SPRITE_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"

class Sprite
{
public:
    Sprite(Vector2I position_screenspace, float rotation, Vector2F scale, Texture* texture);

    virtual void render(Shader& sprite_shader, const Vector2I& viewport_dimensions) const;
    float get_rotation() const;
    void set_rotation(float radians);

    bool operator==(const Sprite& rhs) const;
    Vector2I position_screenspace;
private:
    float rotation;
public:
    Vector2F scale;
private:
    Texture* texture;
    Mesh mesh;
};


#endif //TOPAZ_SPRITE_HPP
