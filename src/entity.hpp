#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "vector.hpp"

class Entity
{
public:
	Entity(Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), Vector3F acceleration = Vector3F());
	virtual void setPosition(Vector3F position);
	void setVelocity(Vector3F velocity);
	void setAcceleration(Vector3F acceleration);
	virtual Vector3F getPosition();
	Vector3F getVelocity();
	Vector3F getAcceleration();
	virtual void updateMotion(unsigned int fps);
protected:
	Vector3F velocity, acceleration;
private:
	Vector3F position;
	//Position is private because a child (Player) will not need an instance of position. getters and setters should mean that other children keep this instance of position.
};

#endif