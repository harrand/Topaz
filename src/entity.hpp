#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "physics.hpp"
#include "object.hpp"
#include <memory>

namespace tz::physics
{
	constexpr float default_mass = 1.0f;
}

/*
	Something which follows the rules of Newtonian Motion. Attach this to something you want to be able to experience motion and forces, such as gravity or thrust.
*/
class Entity
{
public:
	Entity(float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	Entity(const Entity& copy) = default;
	Entity(Entity&& move) = default;
	Entity& operator=(const Entity& rhs) = default;
	
	float get_mass() const;
	virtual const Vector3F& get_position() const;
	virtual void set_position(Vector3F position);
	const Vector3F& get_velocity() const;
	void set_velocity(Vector3F velocity);
	Vector3F get_acceleration() const;
	const std::unordered_map<std::string, Force>& get_forces() const;
	void apply_force(std::string force_name, Force f);
	void remove_force(std::string force_name);
	virtual void update_motion(unsigned int fps);
	bool operator==(const Entity& rhs) const;
protected:
	float mass;
	Vector3F velocity;
	std::unordered_map<std::string, Force> forces;
private:
	Vector3F position;
};

/*
	Essentially an Entity which has a renderable component in the form of an Object. See Object documentation for additional details.
*/
class EntityObject : public Entity, public Object
{
public:
	EntityObject(std::string mesh_link, std::map<Texture::TextureType, std::string> textures, float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), unsigned int shininess = tz::graphics::default_shininess, float parallax_map_scale = tz::graphics::default_parallax_map_scale, float parallax_map_offset = tz::graphics::default_parallax_map_offset, float displacement_factor = tz::graphics::default_displacement_factor, Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	EntityObject(const EntityObject& copy) = default;
	EntityObject(EntityObject&& move) = default;
	EntityObject& operator=(const EntityObject& rhs) = default;
	
	virtual const Vector3F& get_position() const override;
	virtual void set_position(Vector3F position) override;
	bool operator==(const EntityObject& rhs) const;
};

#endif