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

Camera DirectionalLight::get_view() const
{
    //	Camera(Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), float fov = tz::graphics::default_fov, float near_clip = tz::graphics::default_near_clip, float far_clip = tz::graphics::default_far_clip, bool perspective = true);
    Camera camera = {{}, this->direction.normalised() * 2.0f * tz::utility::numeric::consts::pi, tz::graphics::default_fov, tz::graphics::default_near_clip, tz::graphics::default_far_clip, false};
    return camera;
}

PointLight::PointLight(Vector3F position, Vector3F colour, float power): Light(colour, power), position(position) {}