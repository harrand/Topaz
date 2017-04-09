#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "utility.hpp"
#include <memory>

class Entity
{
public:
	Entity(float mass = 1.0f, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	virtual void setPosition(Vector3F position);
	void setVelocity(Vector3F velocity);
	void applyForce(std::string forceName, Force f);
	void removeForce(std::string forceName);
	virtual const Vector3F& getPosition() const;
	float getMass() const;
	const Vector3F& getVelocity() const;
	const Vector3F getAcceleration() const;
	const std::unordered_map<std::string, Force>& getForces() const;
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