#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include "gl/vertex.hpp"
#include <vector>

namespace tz::gl
{
    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */

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

    inline void sort_indices(IndexedMesh& mesh, tz::Vec3 closest_to)
    {
        // Sorts triangles based upon their euclidean distance to closest_to.
        struct Triangle
        {
            Triangle(unsigned int a, unsigned int b, unsigned int c): a(a), b(b), c(c){}
            unsigned int a, b, c;
        };
        auto get_pos = [&mesh](unsigned int index)->tz::Vec3{return mesh.vertices[index].position;};
        auto get_triangle_pos = [&mesh, get_pos](const Triangle& t)
        {
            return (get_pos(t.a) + get_pos(t.b) + get_pos(t.c)) / 3.0f;
        };

        // Get a list of triangles instead, and sort that (keeps indices grouped together in 3s)
        std::vector<Triangle> triangles;
        triangles.reserve(mesh.indices.size() / 3);
        for(std::size_t i = 0; i < mesh.indices.size(); i += 3)
        {
            triangles.emplace_back(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]);
        }

        // Utility lambdas for getting the distance between a triangle and another position vector.
        auto dist = [closest_to](tz::Vec3 position){return (closest_to - position).length();};
        auto triangle_less = [get_triangle_pos, dist](Triangle lhs, Triangle rhs)
        {
            return dist(get_triangle_pos(lhs)) < dist(get_triangle_pos(rhs));
        };

        // Perform the sort via the utility lambda.
        std::sort(triangles.begin(), triangles.end(), triangle_less);
        {
            std::size_t i = 0;
            for(std::size_t j = 0; j < mesh.indices.size(); j += 3)
            {
                const Triangle& cur_tri = triangles[i++];
                mesh.indices[j] = cur_tri.a;
                mesh.indices[j + 1] = cur_tri.b;
                mesh.indices[j + 2] = cur_tri.c;
            }
        }
    }

    /**
     * @}
     */
}

#endif // TOPAZ_GL_MESH_HPP