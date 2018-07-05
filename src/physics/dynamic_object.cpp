#include "dynamic_object.hpp"

DynamicObject::DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces): StaticObject(transform, asset), PhysicsObject(mass, velocity, moment_of_inertia, angular_velocity, forces){}

void DynamicObject::update(float delta_time)
{
    PhysicsObject::update(delta_time);
    this->transform.position += (this->velocity * delta_time);
    this->transform.rotation += (this->angular_velocity * delta_time);
}