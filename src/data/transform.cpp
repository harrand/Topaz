#include "data/transform.hpp"

Transform::Transform(Vector3F position, Vector3F rotation, Vector3F scale): position(position), rotation(rotation), scale(scale){}

Matrix4x4 Transform::model() const
{
    return tz::transform::model(this->position, this->rotation, this->scale);
}

bool Transform::operator==(const Transform &rhs) const
{
    return this->position == rhs.position && this->rotation == rhs.rotation && this->scale == rhs.scale;
}
