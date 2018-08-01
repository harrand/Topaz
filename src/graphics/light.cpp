#include "light.hpp"

Light::Light(Vector3F colour, float power): colour(colour), power(power)
{
    this->set_colour(colour);
    this->set_power(power);
}

const Vector3F& Light::get_colour()const
{
    return this->colour;
}

float Light::get_power() const
{
    return this->power;
}

void Light::set_colour(Vector3F colour)
{
    this->colour = std::clamp(colour, Vector3F{}, Vector3F{1, 1, 1});
}

void Light::set_power(float power)
{
    this->power = std::abs(power);
}

DirectionalLight::DirectionalLight(Vector3F direction, Vector3F colour, float power): Light(colour, power), direction(direction)
{
    this->set_direction(direction);
}

const Vector3F& DirectionalLight::get_direction() const
{
    return this->direction;
}

void DirectionalLight::set_direction(Vector3F direction)
{
    this->direction = direction.normalised();
}

Camera DirectionalLight::get_view(const AABB& scene_boundary) const
{
    //	Camera(Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), float fov = tz::graphics::default_fov, float near_clip = tz::graphics::default_near_clip, float far_clip = tz::graphics::default_far_clip, bool perspective = true);
    Vector3F inverse_direction = this->direction * -1.0f;
    Matrix4x4 view = tz::transform::look_at(inverse_direction, {});
    Camera camera = {inverse_direction, tz::transform::decompose_rotation(view).zyx(), tz::utility::numeric::consts::pi * 1.1f, tz::graphics::default_near_clip, tz::graphics::default_far_clip};
    Vector3F delta = (scene_boundary.get_maximum() - scene_boundary.get_minimum());
    float size = std::max(std::initializer_list<float>{delta.x, delta.y, delta.z});
    camera.set_orthographic(size, -size, size, -size, -size, size);
    return camera;
}

PointLight::PointLight(Vector3F position, Vector3F colour, float power): Light(colour, power), position(position) {}