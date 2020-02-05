#ifndef TOPAZ_GL_MESH_DATA_HPP
#define TOPAZ_GL_MESH_DATA_HPP
#include <vector>
#include "gl/vertex.hpp"

namespace tz::gl
{
    struct MeshData
    {
        std::vector<tz::gl::Vertex> vertices;
    };

    struct MeshIndices
    {
        std::vector<unsigned int> indices;
    };

    struct StandardDataRegionNames
    {
        std::string positions_name;
        std::string texcoords_name;
        std::string normals_name;
        std::string tangents_name;
        std::string bi_tangents_name;
    };

    struct Mesh
    {
        MeshData data;
        MeshIndices indices;
        StandardDataRegionNames data_names;
        std::string indices_names;
    };
}

#endif // TOPAZ_GL_MESH_DATA_HPP