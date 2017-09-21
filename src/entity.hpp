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

#endif