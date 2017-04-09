#include "entity.hpp"

Entity::Entity(float mass, Vector3F position, Vector3F velocity, std::unordered_map<std::string, Force> forces): mass(mass), position(position), velocity(velocity), forces(forces){}

void Entity::setPosition(Vector3F position)
{
	this->position = position;
}

void Entity::setVelocity(Vector3F velocity)
{
	this->velocity = velocity;
}

void Entity::applyForce(std::string forceName, Force f)
{
	this->forces[forceName] = f;
}

void Entity::removeForce(std::string forceName)
{
	this->forces.erase(forceName);
}

const Vector3F& Entity::getPosition() const
{
	return this->position;
}

float Entity::getMass() const
{
	return this->mass;
}

const Vector3F& Entity::getVelocity() const
{
	return this->velocity;
}

const Vector3F Entity::getAcceleration() const
{
	Force resultant;
	for(const auto &ent: this->forces)
	{
		resultant += ent.second;
	}
	
	// fnet = ma, so a = fnet/m
	return Vector3F(resultant.getSize() / this->mass);
}

const std::unordered_map<std::string, Force>& Entity::getForces() const
{
	return this->forces;
}

void Entity::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->position += (velocity / fps);
}