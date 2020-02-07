#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include "gl/vertex.hpp"
#include <vector>

namespace tz::gl
{
    struct IndexedMeshElement
    {
        tz::gl::Vertex vertex;
        tz::gl::Index index;
    };

    struct Mesh
    {
        std::vector<tz::gl::Vertex> vertices;
    };

    struct IndexedMesh
    {
        std::vector<IndexedMeshElement> elements;
    };
}

#endif // TOPAZ_GL_MESH_HPP