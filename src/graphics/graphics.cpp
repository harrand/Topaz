#include "graphics/graphics.hpp"

Vertex::Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal, Vector3F tangent): position(std::move(position)), texture_coordinate(std::move(texture_coordinate)), normal(std::move(normal)), tangent(std::move(tangent)){}

bool Vertex::operator==(const Vertex& rhs) const
{
	return
		this->position == rhs.position &&
		this->texture_coordinate == rhs.texture_coordinate &&
		this->normal == rhs.normal &&
		this->tangent == rhs.tangent;
}