#include "entityobject.hpp"

EntityObject::EntityObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, float mass, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor, Vector3F velocity, std::unordered_map<std::string, Force> forces): Entity(mass, position, velocity, forces), Object(mesh_link, textures, position, rotation, scale, shininess, parallax_map_scale, parallax_map_offset, displacement_factor){}

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