//
// Created by Harrand on 10/08/2019.
//

#ifndef TOPAZ_ASTEROID_HPP
#define TOPAZ_ASTEROID_HPP
#include "physics/dynamic_object.hpp"

enum class AsteroidType
{
    IGNEOUS,
    SEDIMENTARY,
    METALLIC,
    GREEN_BULLET,
    RED_BULLET,
    BLUE_BULLET
};

class Asteroids;

class Asteroid : public DynamicObject
{
public:
    Asteroid(float mass, Transform transform, Asset asset, Vector3F velocity = {}, float moment_of_inertia = 1.0f, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {}, AsteroidType type = AsteroidType::IGNEOUS);
    virtual void on_collision([[maybe_unused]] PhysicsObject &collided) override;
    void set_parent(Asteroids* scene);
    void set_type(AsteroidType type);
private:
    Asteroids* parent;
    AsteroidType type;
};


#endif //TOPAZ_ASTEROID_HPP
