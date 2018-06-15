//
// Created by Harrand on 15/06/2018.
//

#include "physics_object.hpp"

PhysicsObject::PhysicsObject(float mass, Vector3F velocity, Vector3F angular_velocity, std::initializer_list<Vector3F> forces): mass(mass), velocity(velocity), angular_velocity(angular_velocity), forces(forces){}

Vector3F PhysicsObject::get_net_force() const
{
    return std::accumulate(this->forces.begin(), this->forces.end(), Vector3F());
}

Vector3F PhysicsObject::get_acceleration() const
{
    return this->get_net_force() / this->mass;
}

void PhysicsObject::update(float delta_time)
{
    this->velocity += (this->get_acceleration() * delta_time);
}

void PhysicsObject::add_force(Vector3F force)
{
    this->forces.push_back(force);
}