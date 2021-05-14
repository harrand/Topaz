#ifndef TOPAZ_GL_MESH_HPP
#define TOPAZ_GL_MESH_HPP
#include "core/vector.hpp"
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include "gl/renderer.hpp"

namespace tz::gl
{
    /**
     * @brief Represents the typical vertex type Topaz expects.
     */
    struct Vertex
    {
        tz::Vec3 position = {};
        tz::Vec2 texcoord = {};
        tz::Vec3 normal = {};
        tz::Vec3 tangent = {};
        tz::Vec3 bitangent = {};
    };

    struct Mesh
    {
        tz::BasicList<Vertex> vertices;
        tz::BasicList<unsigned int> indices;
    };

    enum class MeshInputIgnoreFlag
    {
        PositionIgnore,
        TexcoordIgnore,
        NormalIgnore,
        TangentIgnore,
        BitangentIgnore
    };

    using MeshInputIgnoreField = tz::EnumField<MeshInputIgnoreFlag>;

    class MeshInput : public IRendererInput
    {
    public:
        MeshInput(Mesh mesh);
        MeshInput(Mesh mesh, MeshInputIgnoreField ignores);

        virtual RendererElementFormat get_format() const final;
        virtual std::span<const std::byte> get_vertex_bytes() const final;
        virtual std::span<const unsigned int> get_indices() const final;
    private:
        Mesh mesh;
        MeshInputIgnoreField ignores;
    };
}

#endif // TOPAZ_GL_MESH_HPP