#include "physics/dynamic_object.hpp"
#include "physics/physics.hpp"

DynamicObject::DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces): StaticObject(transform, asset), PhysicsObject(mass, velocity, moment_of_inertia, angular_velocity, forces), bound_modelspace(tz::physics::bound_aabb(*this->asset.mesh)){}

void DynamicObject::update(float delta_time)
{
    PhysicsObject::update(delta_time);
    this->transform.position += (this->velocity * delta_time);
    this->transform.rotation += (this->angular_velocity * delta_time);
}

AABB DynamicObject::get_boundary() const
{
    return this->bound_modelspace * this->transform.model();
}

void DynamicObject::on_collision(PhysicsObject &collided)
{
    std::cout << "collision\n";
    this->velocity *= -1.1f;
    collided.velocity *= -1.1f;
}