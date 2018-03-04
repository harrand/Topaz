#include "sprite.hpp"

Sprite::Sprite(Vector2F position, float rotation, Vector2F scale, const Texture* texture): Object2D(position, rotation, scale, Vector4F(0.0f, 0.0f, 0.0f, 1.0f)), texture(texture){}

Sprite::Sprite(Vector2F position, float rotation, Vector2F scale, Vector4F colour): Object2D(position, rotation, scale, colour), texture(nullptr){}

Sprite& Sprite::operator=(const Sprite& copy)
{
    this->position = copy.position;
    this->scale = copy.scale;
    this->rotation = copy.rotation;
    this->colour = copy.colour;
    this->texture = copy.texture;
    return *this;
}

void Sprite::render(const Camera& cam, Shader* shader, float width, float height) const
{
    shader->bind();
    shader->set_uniform<Matrix4x4>("m", tz::transform::model(Vector3F(this->position, 0.0f), Vector3F(0.0f, 0.0f, this->rotation), Vector3F(this->scale, 1.0f)));
    shader->set_uniform<Matrix4x4>("v", tz::transform::view(cam.position, cam.rotation));
    shader->set_uniform<Matrix4x4>("p", cam.projection(width, height));
    shader->set_uniform<Vector4F>("colour", this->colour);
    shader->set_uniform<bool>("has_texture", this->texture != nullptr);
    shader->update();
    if(this->texture != nullptr)
        texture->bind(shader, 0);
    this->quad.render(false);
}
