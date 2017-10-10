#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "physics.hpp"
#include "object.hpp"
#include <memory>

namespace tz::physics
{
	constexpr float default_mass = 1.0f;
}

class Entity
{
public:
	Entity(float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	Entity(const Entity& copy) = default;
	Entity(Entity&& move) = default;
	Entity& operator=(const Entity& rhs) = default;
	
	float getMass() const;
	virtual const Vector3F& getPosition() const;
	virtual void setPosition(Vector3F position);
	const Vector3F& getVelocity() const;
	void setVelocity(Vector3F velocity);
	Vector3F getAcceleration() const;
	const std::unordered_map<std::string, Force>& getForces() const;
	void applyForce(std::string force_name, Force f);
	void removeForce(std::string force_name);
	virtual void updateMotion(unsigned int fps);
protected:
	float mass;
	Vector3F velocity;
	std::unordered_map<std::string, Force> forces;
private:
	Vector3F position;
};

class EntityObject : public Entity, public Object
{
public:
	EntityObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, float mass = tz::physics::default_mass, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), unsigned int shininess = tz::graphics::default_shininess, float parallax_map_scale = tz::graphics::default_parallax_map_scale, float parallax_map_offset = tz::graphics::default_parallax_map_offset, float displacement_factor = tz::graphics::default_displacement_factor, Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	EntityObject(const EntityObject& copy) = default;
	EntityObject(EntityObject&& move) = default;
	EntityObject& operator=(const EntityObject& rhs) = default;
	
	virtual const Vector3F& getPosition() const override;
	virtual void setPosition(Vector3F position) override;
};

#endif