#ifndef TOPAZ_TRANSFORM_HPP
#define TOPAZ_TRANSFORM_HPP

#include "vector.hpp"
#include "matrix.hpp"

class Transform
{
public:
    Transform(Vector3F position, Vector3F rotation, Vector3F scale);
    Matrix4x4 model() const;
    Vector3F position, rotation, scale;
};

#endif //TOPAZ_TRANSFORM_HPP
