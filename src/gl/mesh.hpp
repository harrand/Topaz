#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include "gl/vertex.hpp"
#include <vector>

namespace tz::gl
{

    struct Mesh
    {
        std::vector<tz::gl::Vertex> vertices;
    };

    struct IndexedMesh
    {
        std::vector<tz::gl::Vertex> vertices;
        std::vector<tz::gl::Index> indices;

        std::size_t data_size_bytes() const
        {
            return this->vertices.size() * sizeof(tz::gl::Vertex);
        }

        std::size_t indices_size_bytes() const
        {
            return this->indices.size() * sizeof(tz::gl::Index);
        }
    };
}

#endif // TOPAZ_GL_MESH_HPP