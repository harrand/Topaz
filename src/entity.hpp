#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "physics/physics.hpp"
#include "graphics/object.hpp"
#include <memory>

namespace tz::physics
{
	constexpr float default_mass = 1.0f;
}

/**
* Something which follows the rules of Newtonian Motion.
* Attach this to something you want to be able to experience motion and forces, such as gravity or thrust.
*/
class Entity
{
public:
	Entity(float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	Entity(const Entity& copy) = default;
	Entity(Entity&& move) = default;
	Entity& operator=(const Entity& rhs) = default;
	
	Vector3F get_acceleration() const;
	const std::unordered_map<std::string, Force>& get_forces() const;
	/**
	* Apply a force on this object, provided a name.
	* Complexity: O(n) Ω(1) ϴ(1), where n = number of existing forces.
	*/
	void apply_force(std::string force_name, Force f);
	/**
	* Remove the force on this object with the specified name.
	* Complexity: O(n) Ω(1) ϴ(1), where n = number of existing forces
	*/
	void remove_force(std::string force_name);
	virtual void update_motion(unsigned int fps);
	bool operator==(const Entity& rhs) const;
	
	float mass;
	Vector3F position;
	Vector3F velocity;
protected:
	std::unordered_map<std::string, Force> forces;
};

/**
* Essentially an Entity which has a renderable component in the form of an Object3D. See Object3D documentation for additional details.
*/
class EntityObject : public Entity, public Object
{
public:
    /**
     * Manual construction of a new EntityObject.
     * @param mesh - Pointer to read-only Mesh. This must outlive the EntityObject or UB will be invoked.
     * @param textures - Map of each texture-type to the corresponding pointer to Texture. All must outlive the EntityObject or UB will be invoked.
     * @param mass - Desired mass of the EntityObject.
     * @param position - Position of the EntityObject, in world-space.
     * @param rotation - Rotation of the EntityObject, in euler-angles.
     * @param scale - Scale of the EntityObject.
     * @param velocity
     * @param forces
     */
	EntityObject(const Mesh* mesh, Material material, float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	EntityObject(const Object& static_object, float mass);
	EntityObject(const EntityObject& copy) = default;
	EntityObject(EntityObject&& move) = default;
	EntityObject& operator=(const EntityObject& rhs) = default;
	
	virtual void update_motion(unsigned int fps) override;
	
	bool operator==(const EntityObject& rhs) const;
	using Object::position;
};

#endif