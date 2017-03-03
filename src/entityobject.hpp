#ifndef ENTITYOBJECT_HPP
#define ENTITYOBJECT_HPP
#include "object.hpp"
#include "entity.hpp"

class EntityObject: public Entity, public Object
{
public:
	EntityObject(std::string meshLink, std::string textureLink, float mass = 1.0f, Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), Vector3F scale = Vector3F(1, 1, 1), Vector3F velocity = Vector3F(), std::map<std::string, Force> forces = std::map<std::string, Force>());
	void setPosition(Vector3F position);
	Vector3F getPosition();
	void updateMotion(unsigned int fps);
private:
};

#endif