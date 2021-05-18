#include "gl/mesh.hpp"

namespace tz::gl
{
    MeshInput::MeshInput(Mesh mesh):
    MeshInput(mesh, MeshInputIgnoreField{})
    {

    }

    MeshInput::MeshInput(Mesh mesh, MeshInputIgnoreField ignores):
    mesh(mesh),
    ignores(ignores)
    {

    }

    RendererElementFormat MeshInput::get_format() const
    {
        tz::BasicList<RendererAttributeFormat> attributes;
        // Create the attribute formats if they're not ignored.

        if(!this->ignores.contains(MeshInputIgnoreFlag::PositionIgnore))
        {
            attributes.add(
            {
                .element_attribute_offset = offsetof(Vertex, position),
                .type = tz::gl::RendererComponentType::Float32x3
            });
        }

        if(!this->ignores.contains(MeshInputIgnoreFlag::TexcoordIgnore))
        {
            attributes.add(
            {
                .element_attribute_offset = offsetof(Vertex, texcoord),
                .type = tz::gl::RendererComponentType::Float32x2
            });
        }

        if(!this->ignores.contains(MeshInputIgnoreFlag::NormalIgnore))
        {
            attributes.add(
            {
                .element_attribute_offset = offsetof(Vertex, normal),
                .type = tz::gl::RendererComponentType::Float32x3
            });
        }

        if(!this->ignores.contains(MeshInputIgnoreFlag::TangentIgnore))
        {
            attributes.add(
            {
                .element_attribute_offset = offsetof(Vertex, tangent),
                .type = tz::gl::RendererComponentType::Float32x3
            });
        }

        if(!this->ignores.contains(MeshInputIgnoreFlag::BitangentIgnore))
        {
            attributes.add(
            {
                .element_attribute_offset = offsetof(Vertex, bitangent),
                .type = tz::gl::RendererComponentType::Float32x3
            });
        }
        return
        {
            .binding_size = sizeof(Vertex),
            .basis = tz::gl::RendererInputFrequency::PerVertexBasis,
            .binding_attributes = attributes
        };
    }

    std::span<const std::byte> MeshInput::get_vertex_bytes() const
    {
        return std::as_bytes(std::span<const Vertex>(this->mesh.vertices.begin(), this->mesh.vertices.end()));
    }

    std::span<const unsigned int> MeshInput::get_indices() const
    {
        return {this->mesh.indices.begin(), this->mesh.indices.end()};
    }
}