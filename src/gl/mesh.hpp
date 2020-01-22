#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include <vector>
#include "gl/vertex.hpp"

namespace tz::gl
{
    struct Mesh
    {
        std::vector<tz::gl::Vertex> vertices;
    };
}

#endif // TOPAZ_GL_MESH_HPP