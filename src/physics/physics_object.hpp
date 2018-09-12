#ifndef TOPAZ_PHYSICS_OBJECT_HPP
#define TOPAZ_PHYSICS_OBJECT_HPP

#include "data/vector.hpp"
#include "physics/boundary.hpp"
#include "physics.hpp"
#include <map>

class PhysicsObject
{
public:
    PhysicsObject(float mass, Vector3F velocity = {}, float moment_of_inertia = 1.0f, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {}, std::initializer_list<Vector3F> torques = {});
    /**
     * Get the net-force acting on this PhysicsObject.
     * @return - The net-force, in kilograms world-units per second squared (kg w s^(-2)).
     */
    Vector3F net_force() const;
    /**
     * Get the net-torque acting on this PhysicsObject.
     * @return - The net-torque, in newton-metres.
     */
    Vector3F net_torque() const;
    /**
     * Get the acceleration of this PhysicsObject.
     * @return - The acceleration, in  world-units per second squared (w s^(-2)).
     */
    Vector3F get_acceleration() const;
    /**
     * Get the rotational acceleration of this PhysicsObject.
     * @return - The rotational acceleration, in radians per second squared.
     */
    Vector3F get_angular_acceleration() const;
    /**
     * Updates the velocity with the delta
     */
    virtual void update(float delta_time);

    virtual std::optional<AABB> get_boundary() const = 0;
    virtual void on_collision(PhysicsObject& collided) = 0;

    void add_force(Vector3F force);
    void add_torque(Vector3F torque);
    void clear_forces();
    void clear_torques();

    friend class Scene;

    /// Mass, in kilograms.
    float mass;
    /// Velocity, in world-units per second.
    Vector3F velocity;
    /// Moment of inertia, in kilogram metre squared.
    float moment_of_inertia;
    /// Angular-velocity, in radians per second.
    Vector3F angular_velocity;
protected:
    /// Container of all the forces acting upon the PhysicsObject.
    std::vector<Vector3F> forces;
    /// Container of all the torques (rotational forces) acting upon the PhysicsObject.
    std::vector<Vector3F> torques;
private:
    std::vector<PhysicsObject*> colliding_with;

    void handle_collisions(const std::vector<std::reference_wrapper<PhysicsObject>>& physics_objects);
    void handle_collisions_sort_and_sweep(tz::physics::Axis2D highest_variance_axis, const std::multimap<float, std::reference_wrapper<PhysicsObject>>& physics_objects_sorted);
    void handle_collisions_sort_and_sweep(tz::physics::Axis3D highest_variance_axis, const std::multimap<float, std::reference_wrapper<PhysicsObject>>& physics_objects_sorted);
};


#endif //TOPAZ_PHYSICS_OBJECT_HPP
