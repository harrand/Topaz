#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "physics.hpp"
#include "object.hpp"
#include <memory>

class Entity
{
public:
	Entity(float mass = 1.0f, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	Entity(const Entity& copy) = default;
	Entity(Entity&& move) = default;
	Entity& operator=(const Entity& rhs) = default;
	
	float getMass() const;
	void applyForce(std::string force_name, Force f);
	void removeForce(std::string force_name);
	virtual const Vector3F& getPosition() const;
	virtual Vector3F& getPositionR();
	const Vector3F& getVelocity() const;
	Vector3F& getVelocityR();
	Vector3F getAcceleration() const;
	const std::unordered_map<std::string, Force>& getForces() const;
	std::unordered_map<std::string, Force>& getForcesR();
	virtual void updateMotion(unsigned int fps);
protected:
	float mass;
	Vector3F velocity;
	std::unordered_map<std::string, Force> forces;
private:
	Vector3F position;
};

class EntityObject: public Entity, public StaticObject
{
public:
	EntityObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, float mass = 1.0f, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), unsigned int shininess = 5, float parallax_map_scale = 0.04f, float parallax_map_offset = -0.5f, float displacement_factor = 0.25f, Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	EntityObject(const EntityObject& copy) = default;
	EntityObject(EntityObject&& move) = default;
	EntityObject& operator=(const EntityObject& rhs) = default;
	
	const Vector3F& getPosition() const;
	Vector3F& getPositionR();
	void updateMotion(unsigned int fps);
};

#endif