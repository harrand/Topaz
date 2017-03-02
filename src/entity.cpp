#include "entity.hpp"

Entity::Entity(float mass, Vector3F position, Vector3F velocity, std::map<std::string, Force> forces): mass(mass), position(position), velocity(velocity), forces(forces){}

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
	for(const auto &ent: this->forces)
	{
		resultant += ent.second;
	}
	/*
	for(unsigned int i = 0; i < this->forces.size(); i++)
	{
		resultant += this->forces.at(i);
	}
	*/
	
	// fnet = ma, so a = fnet/m
	return Vector3F(resultant.getSize() / this->mass);
}

std::map<std::string, Force> Entity::getForces()
{
	return this->forces;
}

void Entity::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->position += (velocity / fps);
}