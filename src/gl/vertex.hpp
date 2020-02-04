#ifndef TOPAZ_GL_VERTEX_HPP
#define TOPAZ_GL_VERTEX_HPP
#include "geo/vector.hpp"
#include <vector>

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

    struct VertexList
    {
        VertexList(const Vertex* vertex_data, std::size_t vertices_size);
        std::size_t size() const;
        std::size_t size_bytes() const;

        std::vector<tz::Vec3> positions;
        std::vector<tz::Vec2> texture_coordinates;
        std::vector<tz::Vec3> normals;
        std::vector<tz::Vec3> tangents;
        std::vector<tz::Vec3> bi_tangents;
    };
}

#endif // TOPAZ_GL_VERTEX_HPP