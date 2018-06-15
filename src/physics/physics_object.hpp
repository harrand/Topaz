#include "../vector.hpp"

#ifndef TOPAZ_PHYSICS_OBJECT_HPP
#define TOPAZ_PHYSICS_OBJECT_HPP

class PhysicsObject
{
public:
    PhysicsObject(float mass, Vector3F velocity = {}, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {});
    /**
     * Get the net-force acting on this PhysicsObject.
     * @return - The net-force, in kilograms world-units per second squared (kg w s^(-2)).
     */
    Vector3F get_net_force() const;
    /**
     * Get the acceleration of this PhysicsObject.
     * @return - The acceleration, in  world-units per second squared (w s^(-2)).
     */
    Vector3F get_acceleration() const;
    /**
     * Updates the velocity with the delta
     */
    virtual void update(float delta_time);

    void add_force(Vector3F force);

    /// Mass, in kilograms.
    float mass;
    /// Velocity, in world-units per second.
    Vector3F velocity;
    /// Angular-velocity, in radians per second.
    Vector3F angular_velocity;
protected:
    /// Container of all the forces acting upon the PhysicsObject.
    std::vector<Vector3F> forces;
};


#endif //TOPAZ_PHYSICS_OBJECT_HPP
