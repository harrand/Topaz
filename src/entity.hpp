#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "utility.hpp"
#include <memory>

class Entity
{
public:
	Entity(float mass = 1.0f, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	Entity(const Entity& copy) = default;
	Entity(Entity&& move) = default;
	Entity& operator=(const Entity& rhs) = default;
	
	float getMass() const;
	
	virtual void setPosition(Vector3F position);
	void setVelocity(Vector3F velocity);
	
	void applyForce(std::string force_name, Force f);
	void removeForce(std::string force_name);
	
	virtual Vector3F& getPositionR();
	virtual const Vector3F& getPosition() const;
	
	const Vector3F& getVelocity() const;
	Vector3F& getVelocityR();
	
	const Vector3F getAcceleration() const;
	
	const std::unordered_map<std::string, Force>& getForces() const;
	std::unordered_map<std::string, Force>& getForcesR();
	
	virtual void updateMotion(unsigned int fps);
protected:
	float mass;
	Vector3F velocity;
	std::unordered_map<std::string, Force> forces;
private:
	Vector3F position;
	//Position is private because a child (Player) will not need an instance of position. getters and setters should mean that other children keep this instance of position.
};

#endif