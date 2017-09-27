#ifndef ENTITYOBJECT_HPP
#define ENTITYOBJECT_HPP
#include "object.hpp"
#include "entity.hpp"

class EntityObject: public Entity, public Object
{
public:
	EntityObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, float mass = 1.0f, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), unsigned int shininess = 5, float parallax_map_scale = 0.04f, float parallax_map_offset = -0.5f, float displacement_factor = 0.25f, Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	EntityObject(const EntityObject& copy) = default;
	EntityObject(EntityObject&& move) = default;
	EntityObject& operator=(const EntityObject& rhs) = default;
	
	const Vector3F& getPosition() const;
	Vector3F& getPositionR();
	void updateMotion(unsigned int fps);
};

#endif