#include "physics_object.hpp"


PhysicsObject::PhysicsObject(float mass, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces, std::initializer_list<Vector3F> torques): mass(mass), velocity(velocity), moment_of_inertia(moment_of_inertia), angular_velocity(angular_velocity), forces(forces), torques(torques){}

Vector3F PhysicsObject::net_force() const
{
    return std::accumulate(this->forces.begin(), this->forces.end(), Vector3F{});
}

Vector3F PhysicsObject::net_torque() const
{
    return std::accumulate(this->torques.begin(), this->torques.end(), Vector3F{});
}

Vector3F PhysicsObject::get_acceleration() const
{
    return this->net_force() / this->mass;
}

Vector3F PhysicsObject::get_angular_acceleration() const
{
    return this->net_torque() / this->moment_of_inertia;
}

void PhysicsObject::update(float delta_time)
{
    this->velocity += (this->get_acceleration() * delta_time);
    this->angular_velocity += (this->get_angular_acceleration() * delta_time);
}

void PhysicsObject::add_force(Vector3F force)
{
    this->forces.push_back(force);
}

void PhysicsObject::add_torque(Vector3F torque)
{
    this->torques.push_back(torque);
}

void PhysicsObject::handle_collisions(const std::vector<std::reference_wrapper<PhysicsObject>>& physics_objects)
{
    for(auto ref : physics_objects)
    {
        auto& physics_object = ref.get();
        if(this->get_boundary().intersects(physics_object.get_boundary()) && this != &physics_object)
            this->on_collision(physics_object);
    }
}