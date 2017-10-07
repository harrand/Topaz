#include "entity.hpp"

Entity::Entity(float mass, Vector3F position, Vector3F velocity, std::unordered_map<std::string, Force> forces): mass(mass), velocity(std::move(velocity)), forces(std::move(forces)), position(std::move(position)){}

float Entity::getMass() const
{
	return this->mass;
}

void Entity::applyForce(std::string force_name, Force f)
{
	this->forces[force_name] = f;
}

void Entity::removeForce(std::string force_name)
{
	this->forces.erase(force_name);
}

const Vector3F& Entity::getPosition() const
{
	return this->position;
}

Vector3F& Entity::getPositionR()
{
	return this->position;
}

const Vector3F& Entity::getVelocity() const
{
	return this->velocity;
}

Vector3F& Entity::getVelocityR()
{
	return this->velocity;
}

Vector3F Entity::getAcceleration() const
{
	Force resultant;
	for(const auto &ent: this->forces)
		resultant += ent.second;
	return (resultant.getSize() / this->mass);
}

const std::unordered_map<std::string, Force>& Entity::getForces() const
{
	return this->forces;
}

std::unordered_map<std::string, Force>& Entity::getForcesR()
{
	return this->forces;
}

void Entity::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->position += (velocity / fps);
}

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