#include "entityobject.hpp"

EntityObject::EntityObject(std::string meshLink, std::string textureLink, float mass, Vector3F position, Vector3F rotation, Vector3F scale, Vector3F velocity, std::map<std::string, Force> forces): Entity(mass, position, velocity, forces), Object(meshLink, textureLink, position, rotation, scale)
{
	
}

void EntityObject::setPosition(Vector3F pos)
{
	//this->pos is a protected member in Object. Entity::position is private so is not in scope here.
	this->getPosR() = pos;
}

Vector3F EntityObject::getPosition()
{
	return this->getPos();
}

void EntityObject::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->getPosR() += (this->velocity/fps);
}