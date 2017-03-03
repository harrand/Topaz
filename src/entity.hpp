#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "utility.hpp"
#include <memory>

class Entity
{
public:
	Entity(float mass = 1.0f, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::map<std::string, Force> forces = std::map<std::string, Force>());
	virtual void setPosition(Vector3F position);
	void setVelocity(Vector3F velocity);
	void applyForce(std::string forceName, Force f);
	void removeForce(std::string forceName);
	virtual Vector3F getPosition();
	float getMass();
	Vector3F getVelocity();
	Vector3F getAcceleration();
	std::map<std::string, Force> getForces();
	virtual void updateMotion(unsigned int fps);
protected:
	float mass;
	Vector3F velocity;
	std::map<std::string, Force> forces;
private:
	Vector3F position;
	//Position is private because a child (Player) will not need an instance of position. getters and setters should mean that other children keep this instance of position.
};

#endif