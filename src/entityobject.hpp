#ifndef ENTITYOBJECT_HPP
#define ENTITYOBJECT_HPP
#include "object.hpp"
#include "entity.hpp"

class EntityObject: public Entity, public Object
{
public:
	EntityObject(std::string meshLink, std::string textureLink, std::string normalMapLink, std::string displacementMapLink, float mass = 1.0f, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), Vector3F velocity = Vector3F(), std::unordered_map<std::string, Force> forces = std::unordered_map<std::string, Force>());
	void setPosition(Vector3F position);
	Vector3F getPosition() const;
	void updateMotion(unsigned int fps);
private:
};

#endif