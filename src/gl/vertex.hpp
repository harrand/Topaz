#ifndef TOPAZ_GL_VERTEX_HPP
#define TOPAZ_GL_VERTEX_HPP
#include "geo/vector.hpp"

namespace tz::gl
{
    struct Vertex
    {
        tz::Vec3 position;
        tz::Vec2 texture_coordinate;
        tz::Vec3 normal;
        tz::Vec3 tangent;
        tz::Vec3 bi_tangent;
    };
}

#endif // TOPAZ_GL_VERTEX_HPP