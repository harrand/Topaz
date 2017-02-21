#include "entity.hpp"

Entity::Entity(std::shared_ptr<World>& world, float mass, Vector3F position, Vector3F velocity, std::vector<Force> forces): world(world), mass(mass), position(position), velocity(velocity), forces(forces)
{
	
}

void Entity::setPosition(Vector3F position)
{
	this->position = position;
}

void Entity::setVelocity(Vector3F velocity)
{
	this->velocity = velocity;
}

void Entity::applyForce(Force f)
{
	this->forces.push_back(f);
}

Vector3F Entity::getPosition()
{
	return this->position;
}

Vector3F Entity::getVelocity()
{
	return this->velocity;
}

Vector3F Entity::getAcceleration()
{
	Force resultant;
	resultant += this->world->getGravity();
	for(unsigned int i = 0; i < this->forces.size(); i++)
	{
		resultant += this->forces.at(i);
	}
	// fnet = ma, so a = fnet/m
	return Vector3F(resultant.getSize() / this->mass);
}

std::vector<Force> Entity::getForces()
{
	return this->forces;
}

void Entity::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->position += (velocity / fps);
}