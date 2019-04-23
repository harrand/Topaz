#ifndef TOPAZ_PHYSICS_OBJECT_HPP
#define TOPAZ_PHYSICS_OBJECT_HPP

#include "data/vector.hpp"
#include "physics/boundary.hpp"
#include "physics.hpp"
#include <map>

class PhysicsObject
{
public:
	/**
	 * Construct a PhysicsObject with the given properties.
	 * @param mass - Mass of the object, in kilograms
	 * @param velocity - Initial velocity of the object, in world-units per second
	 * @param moment_of_inertia - Moment of inertia, in kilogram world-units squared
	 * @param angular_velocity - Initial angular velocity of the object, in radians per second
	 * @param forces - Initial set of forces applying on the object, if any, in newtons
	 * @param torques - Initial set of torques apply on the object, if any, in newton world-units
	 */
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

	/**
	 * Retrieve an AABB bounding the object, if there exists a mesh or model attached.
	 * @return - AABB bounding the object if there is geometry to bound, null if no such geometry exists
	 */
	virtual std::optional<AABB> get_boundary() const = 0;
	/**
	 * Callback function invoked when a collision is detected between this and another distinct object in the same scene.
	 * @param collided - The object colliding with this
	 */
	virtual void on_collision(PhysicsObject& collided) = 0;

	/**
	 * Apply a distinct force on the object.
	 * @param force - Force, in newtons
	 */
	void add_force(Vector3F force);
	/**
	 * Apply a distinct torque on the object.
	 * @param torque - Torque, in newton world-units
	 */
	void add_torque(Vector3F torque);
	/**
	 * Remove all forces applied on the object.
	 */
	void clear_forces();
	/**
	 * Remove all torques applied on the object.
	 */
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
	/// Container of all objects that this object is currently colliding with. Updated per tick.
	std::vector<PhysicsObject*> colliding_with;

	/**
	 * Naively iterate through a collection of reference to PhysicsObject to invoke PhysicsObject::on_collision(...) if collision is detected.
	 * @param physics_objects - Container of all physics objects to perform collision detection with
	 */
	void handle_collisions(const std::vector<std::reference_wrapper<PhysicsObject>>& physics_objects);
	/**
	 * Perform collision detection on a multimap of sorted physics objects, based upon the given 2D highest variance axis. Should be used for objects strictly in 2D circumstances.
	 * @param highest_variance_axis - Axis in which there is the greatest range of displacements between all objects
	 * @param physics_objects_sorted - Container of objects to perform collison detection on, sorted by variance axis
	 */
	void handle_collisions_sort_and_sweep(tz::physics::Axis2D highest_variance_axis, const std::multimap<float, std::reference_wrapper<PhysicsObject>>& physics_objects_sorted);
	/**
	 * Perform collision detection on a multimap of sorted physics objects, based upon the given 2D highest variance axis. Should be used for objects strictly in 3D circumstances.
	 * @param highest_variance_axis - Axis in which there is the greatest range of displacements between all objects
	 * @param physics_objects_sorted - Container of objects to perform collison detection on, sorted by variance axis
	 */
	void handle_collisions_sort_and_sweep(tz::physics::Axis3D highest_variance_axis, const std::multimap<float, std::reference_wrapper<PhysicsObject>>& physics_objects_sorted);
};


#endif //TOPAZ_PHYSICS_OBJECT_HPP
