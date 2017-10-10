#include "entity.hpp"

Entity::Entity(float mass, Vector3F position, Vector3F velocity, std::unordered_map<std::string, Force> forces): mass(mass), velocity(std::move(velocity)), forces(std::move(forces)), position(std::move(position)){}

float Entity::getMass() const
{
	return this->mass;
}

const Vector3F& Entity::getPosition() const
{
	return this->position;
}

void Entity::setPosition(Vector3F position)
{
	this->position = position;
}

const Vector3F& Entity::getVelocity() const
{
	return this->velocity;
}

void Entity::setVelocity(Vector3F velocity)
{
	this->velocity = velocity;
}

Vector3F Entity::getAcceleration() const
{
	// get net force (total) and divide by mass. Newton's second law.
	Force resultant;
	for(const auto &ent: this->forces)
		resultant += ent.second;
	return (resultant.getSize() / this->mass);
}

const std::unordered_map<std::string, Force>& Entity::getForces() const
{
	return this->forces;
}

void Entity::applyForce(std::string force_name, Force f)
{
	this->forces[force_name] = f;
}

void Entity::removeForce(std::string force_name)
{
	this->forces.erase(force_name);
}

void Entity::updateMotion(unsigned int fps)
{
	// basically perform integral numerically
	this->velocity += (this->getAcceleration() / fps);
	this->position += (this->velocity / fps);
}

EntityObject::EntityObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, float mass, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor, Vector3F velocity, std::unordered_map<std::string, Force> forces): Entity(mass, position, velocity, forces), Object(mesh_link, textures, position, rotation, scale, shininess, parallax_map_scale, parallax_map_offset, displacement_factor){}

const Vector3F& EntityObject::getPosition() const
{
	return Object::getPosition();
}

void EntityObject::setPosition(Vector3F position)
{
	Object::setPosition(position);
}