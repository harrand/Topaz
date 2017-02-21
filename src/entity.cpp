#include "entity.hpp"

Entity::Entity(Vector3F position, Vector3F velocity, Vector3F acceleration): position(position), velocity(velocity), acceleration(acceleration)
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

void Entity::setAcceleration(Vector3F acceleration)
{
	this->acceleration = acceleration;
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
	return this->acceleration;
}

void Entity::updateMotion(unsigned int fps)
{
	this->velocity += (acceleration / fps);
	this->position += (velocity / fps);
}