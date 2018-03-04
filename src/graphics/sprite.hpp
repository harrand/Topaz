#include "object.hpp"

#ifndef SPRITE_HPP
#define SPRITE_HPP

/**
* Like a normal Object2D, but has a texture bound.
*/
class Sprite : public Object2D
{
public:
    Sprite(Vector2F position, float rotation, Vector2F scale, const Texture* texture);
    Sprite(Vector2F position, float rotation, Vector2F scale, Vector4F colour);
    Sprite& operator=(const Sprite& copy);
    virtual void render(const Camera& cam, Shader* shader, float width, float height) const override;
private:
    const Texture* texture;
};


#endif //SPRITE_HPP
