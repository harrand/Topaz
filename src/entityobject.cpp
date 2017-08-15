#include "entityobject.hpp"

EntityObject::EntityObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, float mass, Vector3F position, Vector3F rotation, Vector3F scale, Vector3F velocity, std::unordered_map<std::string, Force> forces): Entity(mass, position, velocity, forces), Object(mesh_link, textures, position, rotation, scale){}

void EntityObject::setPosition(Vector3F pos)
{
	//this->pos is a protected member in Object. Entity::position is private so is not visible here.
	this->getPositionR() = pos;
}

const Vector3F& EntityObject::getPosition() const
{
	return Object::getPosition();
}

Vector3F& EntityObject::getPositionR()
{
	return Object::getPositionR();
}

void EntityObject::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->getPositionR() += (this->velocity/fps);
}