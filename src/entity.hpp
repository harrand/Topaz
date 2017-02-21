#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "utility.hpp"
#include "world.hpp"
#include <memory>

class Entity
{
public:
	Entity(std::shared_ptr<World>& world, float mass = 1.0, Vector3F position = Vector3F(), Vector3F velocity = Vector3F(), std::vector<Force> forces = std::vector<Force>());
	virtual void setPosition(Vector3F position);
	void setVelocity(Vector3F velocity);
	void applyForce(Force f);
	virtual Vector3F getPosition();
	Vector3F getVelocity();
	Vector3F getAcceleration();
	std::vector<Force> getForces();
	virtual void updateMotion(unsigned int fps);
protected:
	std::shared_ptr<World>& world;
	float mass;
	Vector3F velocity;
	std::vector<Force> forces;
private:
	Vector3F position;
	//Position is private because a child (Player) will not need an instance of position. getters and setters should mean that other children keep this instance of position.
};

#endif