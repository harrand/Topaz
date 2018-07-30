#ifndef TOPAZ_LIGHT_HPP
#define TOPAZ_LIGHT_HPP

#include "data/matrix.hpp"
#include "graphics/camera.hpp"
#include "physics/boundary.hpp"

class Light
{
public:
    Light(Vector3F colour = {1, 1, 1}, float power = 1.0f);
    const Vector3F& get_colour()const ;
    float get_power() const;
    void set_colour(Vector3F colour);
    void set_power(float power);
private:
    Vector3F colour;
    float power;
};

class DirectionalLight : public Light
{
public:
    DirectionalLight(Vector3F direction = {}, Vector3F colour = {1, 1, 1}, float power = 1.0f);
    const Vector3F& get_direction() const;
    void set_direction(Vector3F direction);
    Camera get_view(const AABB& scene_boundary) const;
private:
    Vector3F direction;
};

class PointLight : public Light
{
public:
    PointLight(Vector3F position = {}, Vector3F colour = {1, 1, 1}, float power = 1.0f);
    Vector3F position;
};


#endif //TOPAZ_LIGHT_HPP
