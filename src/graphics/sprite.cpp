//
// Created by Harry on 17/08/2018.
//

#include "sprite.hpp"

Sprite::Sprite(Vector2F position_screenspace, float rotation, Vector2F scale, const Texture* texture): position_screenspace(position_screenspace), rotation(rotation), scale(scale), texture(texture), mesh(tz::graphics::create_quad(0.0f, 0.0f, 0.5f, -0.5f))
{
    this->set_rotation(this->rotation);
}

void Sprite::render(Shader &sprite_shader, const Vector2I &viewport_dimensions) const
{
    tz::graphics::gui_render_mode();
    sprite_shader.bind();
    sprite_shader.set_uniform<bool>("has_texture", true);
    sprite_shader.set_uniform<bool>("has_background_colour", false);
    Matrix4x4 projection = tz::transform::orthographic_projection(viewport_dimensions.x, 0.0f, viewport_dimensions.y, 0.0f, -1.0f, 1.0f);
    Matrix4x4 model = projection * tz::transform::model({this->position_screenspace, 0.0f}, {0.0f, 0.0f, this->rotation}, {this->scale, 1.0f});
    sprite_shader.set_uniform<Matrix4x4>("model_matrix", model);
    this->texture->bind(&sprite_shader, 0);
    sprite_shader.update();
    this->mesh.render(false);
    tz::graphics::scene_render_mode();
    /*
     shader.bind();
    shader.set_uniform<bool>("has_texture", this->has_texture());
    shader.set_uniform<bool>("has_background_colour", this->has_colour());
    Matrix4x4 projection = tz::transform::orthographic_projection(window_width_pixels, 0.0f, window_height_pixels, 0.0f, -1.0f, 1.0f);
    Matrix4x4 model = projection * tz::transform::model(Vector3F(this->get_x(), this->get_y(), 0.0f) * 0.5f, Vector3F(), Vector3F(this->get_width(), this->get_height(), 0.0f));
    shader.set_uniform<Matrix4x4>("model_matrix", model);
    if(this->has_colour())
        shader.set_uniform<Vector4F>("colour", this->get_colour().value());
    else if(this->has_texture())
    {
        this->get_texture()->bind(&shader, 0);
    }
    shader.update();
    this->mesh.render(false);
     */
}

float Sprite::get_rotation() const
{
    return this->rotation;
}

void Sprite::set_rotation(float radians)
{
    // Maybe put between -pi and pi
    this->rotation = radians;
}

const Texture* Sprite::get_texture() const
{
    return this->texture;
}

void Sprite::set_texture(const Texture* texture)
{
    this->texture = texture;
}

bool Sprite::operator==(const Sprite &rhs) const
{
    return this->position_screenspace == rhs.position_screenspace && this->rotation == rhs.rotation && this->scale == rhs.scale && this->texture == rhs.texture;
}
