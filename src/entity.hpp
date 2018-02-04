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
class EntityObject3D : public Entity, public Object3D
{
public:
	EntityObject3D(const Mesh* mesh, std::map<tz::graphics::TextureType, Texture*> textures, float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), unsigned int shininess = tz::graphics::default_shininess, float parallax_map_scale = tz::graphics::default_parallax_map_scale, float parallax_map_offset = tz::graphics::default_parallax_map_offset, float displacement_factor = tz::graphics::default_displacement_factor, Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	EntityObject3D(const EntityObject3D& copy) = default;
	EntityObject3D(EntityObject3D&& move) = default;
	EntityObject3D& operator=(const EntityObject3D& rhs) = default;
	
	virtual void update_motion(unsigned int fps) override;
	
	bool operator==(const EntityObject3D& rhs) const;
	using Object3D::position;
};

#endif