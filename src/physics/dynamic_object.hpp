#ifndef DYNAMIC_OBJECT_HPP
#define DYNAMIC_OBJECT_HPP
#include "graphics/static_object.hpp"
#include "physics/physics_object.hpp"

/**
 * Like a StaticObject, but has motion integration and collision detection.
 */
class DynamicObject : public StaticObject, public PhysicsObject
{
public:
	/**
	 * Construct a DynamicObject with the given properties.
	 * @param mass - Mass, in kilograms
	 * @param transform - Initial transform of the object
	 * @param asset - Asset used to render the object
	 * @param velocity - Initial velocity of the object, in world-units per second
	 * @param moment_of_inertia - Moment of inertia of the object, in kilogram world-units squared
	 * @param angular_velocity - Initial angular velocity of the object, in radians per second
	 * @param forces - Container of forces applied to the object, if any
	 */
	DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity = {}, float moment_of_inertia = 1.0f, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {});
	DynamicObject(const DynamicObject& copy) = default;
	DynamicObject(DynamicObject&& move) = default;
	virtual ~DynamicObject() = default;
	DynamicObject& operator=(const DynamicObject& rhs) = default;
	DynamicObject& operator=(DynamicObject&& rhs) = default;
	/**
	 * Perform an update. This should be invoked once per tick.
	 * @param delta_time - Time passed since the previous tick, in seconds
	 */
	virtual void update(float delta_time) override;
	/**
	 * Retrieve an AABB bounding the DynamicObject, if geometry exists to bound.
	 * @return - AABB bounding the object if geometry exists, otherwise null
	 */
	virtual std::optional<AABB> get_boundary() const override;
	virtual std::unique_ptr<Renderable> unique_clone() const override;
	/**
	 * Callback function, invoked when a collision is detected between this object and another PhysicsObject.
	 * @param collided - The PhysicsObject that collided with this object
	 */
	virtual void on_collision(PhysicsObject& collided) override;

	friend class InstancedDynamicObject;
};

/**
 * Container of DynamicObjects specifically designed to be rendered in one draw-call. All objects must share deeply-identical assets.
 */
class InstancedDynamicObject : public DynamicObject
{
public:
	/**
	 * Construct an InstancedDynamicObject based upon an existing set of objects
	 * @param objects - DynamicObjects to batch an instance of
	 */
	InstancedDynamicObject(const std::vector<DynamicObject>& objects);
	/**
	 * Retrieve the total mass of all DynamicObjects.
	 * @return - Total mass of the instances, in kilograms
	 */
	float get_mass() const;
	/**
	 * Retrieve the underlying InstancedMesh.
	 * @return - Instanced mesh
	 */
	const InstancedMesh& get_instanced_mesh() const;
	/**
	 * Perform an update. This should be invoked once per tick.
	 * @param delta_time - Time passed since the previous tick, in seconds
	 */
	virtual void update(float delta_time) override;
	/**
	 * Render the DynamicObject instances in a single render-call.
	 * @param render_pass - Render properties of the render-call
	 */
	virtual void render(RenderPass render_pass) const override;
	virtual std::unique_ptr<Renderable> unique_clone() const override;
	/**
	 * Retrieve an AABB bounding ALL DynamicObject instances
	 * @return - Mass boundary of instances
	 */
	virtual std::optional<AABB> get_boundary() const override;
	/**
	 * Invoked when a collision is detected between this container of DynamicObjects (judged by InstancedDynamicObject::get_boundary()) and another object
	 * @param collided - The object which collided with this group of instances
	 */
	virtual void on_collision(PhysicsObject& collided) override;
private:
	using DynamicObject::mass;
	using DynamicObject::get_boundary;
	using DynamicObject::on_collision;
	/// Underlying InstancedMesh
	std::shared_ptr<InstancedMesh> instanced_mesh;
	/// Underlying DynamicObject instances
	std::vector<DynamicObject> objects;
};


#endif //DYNAMIC_OBJECT_HPP
