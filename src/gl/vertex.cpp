#include "gl/vertex.hpp"

namespace tz::gl
{
    VertexList::VertexList(const Vertex* vertex_data, std::size_t vertices_size)
    {
        for(std::size_t i = 0; i < vertices_size; i++)
        {
            const Vertex& v = vertex_data[i];
            this->positions.push_back(v.position);
            this->texture_coordinates.push_back(v.texture_coordinate);
            this->normals.push_back(v.normal);
            this->tangents.push_back(v.tangent);
            this->bi_tangents.push_back(v.bi_tangent);
        }
    }

    std::size_t VertexList::size() const
    {
        return this->positions.size()
            + this->texture_coordinates.size()
            + this->normals.size()
            + this->tangents.size()
            + this->bi_tangents.size();
    }

    std::size_t VertexList::size_bytes() const
    {
        return
              (this->positions.size() * sizeof(float) * 3)
            + (this->texture_coordinates.size() * sizeof(float) * 2)
            + (this->normals.size() * sizeof(float) * 3)
            + (this->tangents.size() * sizeof(float) * 3)
            + (this->bi_tangents.size() * sizeof(float) * 3);
    }
}