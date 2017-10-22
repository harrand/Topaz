#include "entity.hpp"

Entity::Entity(float mass, Vector3F position, Vector3F velocity, std::unordered_map<std::string, Force> forces): mass(mass), velocity(std::move(velocity)), forces(std::move(forces)), position(std::move(position)){}

float Entity::get_mass() const
{
	return this->mass;
}

const Vector3F& Entity::get_position() const
{
	return this->position;
}

void Entity::set_position(Vector3F position)
{
	this->position = position;
}

const Vector3F& Entity::get_velocity() const
{
	return this->velocity;
}

void Entity::set_velocity(Vector3F velocity)
{
	this->velocity = velocity;
}

Vector3F Entity::get_acceleration() const
{
	// get net force (total) and divide by mass. Newton's second law.
	Force resultant;
	for(const auto &ent: this->forces)
		resultant += ent.second;
	return (resultant.get_size() / this->mass);
}

const std::unordered_map<std::string, Force>& Entity::get_forces() const
{
	return this->forces;
}

void Entity::apply_force(std::string force_name, Force f)
{
	this->forces[force_name] = f;
}

void Entity::remove_force(std::string force_name)
{
	this->forces.erase(force_name);
}

void Entity::update_motion(unsigned int fps)
{
	// basically perform integral numerically
	this->velocity += (this->get_acceleration() / fps);
	this->position += (this->velocity / fps);
}

bool Entity::operator==(const Entity& rhs) const
{
	return this->mass == rhs.mass && this->velocity == rhs.velocity && this->forces == rhs.forces && this->position == rhs.position;
}

EntityObject::EntityObject(std::string mesh_link, std::map<Texture::TextureType, std::string> textures, float mass, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor, Vector3F velocity, std::unordered_map<std::string, Force> forces): Entity(mass, position, velocity, forces), Object(mesh_link, textures, position, rotation, scale, shininess, parallax_map_scale, parallax_map_offset, displacement_factor){}

const Vector3F& EntityObject::get_position() const
{
	return Object::get_position();
}

void EntityObject::set_position(Vector3F position)
{
	Object::set_position(position);
}

bool EntityObject::operator==(const EntityObject& rhs) const
{
	return *dynamic_cast<const Object*>(this) == *dynamic_cast<const Object*>(&rhs) && *dynamic_cast<const Entity*>(this) == *dynamic_cast<const Entity*>(&rhs);
}