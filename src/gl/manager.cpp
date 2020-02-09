#include "gl/manager.hpp"

namespace tz::gl
{
    Manager::Manager(): o(), data_handle(o.emplace_buffer<tz::gl::BufferType::Array>()), index_handle(o.emplace_buffer<tz::gl::BufferType::Index>())
    {
        // We have custom formats now.
        std::size_t vertex_stride_bytes = sizeof(tz::gl::Vertex);
        auto to_ptr = [](std::size_t offset)->const void*{return reinterpret_cast<const void*>(offset);};
        // Position
        o.format_custom(this->data_handle, 3, GL_FLOAT, GL_FALSE, vertex_stride_bytes, to_ptr(0));
        // Texcoord
        o.format_custom(this->data_handle, 2, GL_FLOAT, GL_FALSE, vertex_stride_bytes, to_ptr(sizeof(tz::Vec3)));
        // Normal
        o.format_custom(this->data_handle, 3, GL_FLOAT, GL_TRUE, vertex_stride_bytes, to_ptr(sizeof(tz::Vec3) + sizeof(tz::Vec2)));
        // Tangent
        o.format_custom(this->data_handle, 3, GL_FLOAT, GL_TRUE, vertex_stride_bytes, to_ptr(sizeof(tz::Vec3) + sizeof(tz::Vec2) + sizeof(tz::Vec3)));
        // Bitangent
        o.format_custom(this->data_handle, 3, GL_FLOAT, GL_TRUE, vertex_stride_bytes, to_ptr(sizeof(tz::Vec3) + sizeof(tz::Vec2) + sizeof(tz::Vec3) + sizeof(tz::Vec3)));
    }

    Manager::Handle Manager::add_mesh(tz::gl::IndexedMesh data)
    {
        std::size_t cur_offset_bytes = this->data()->size();
        std::size_t cur_offset_vertices = cur_offset_bytes / sizeof(tz::gl::Vertex);
        std::size_t mesh_size_vertices;
        {
            std::size_t append_size = data.data_size_bytes();
            mesh_size_vertices = append_size / sizeof(tz::gl::Vertex);
            // ensure we have append_size extra data to work with.
            this->data()->safe_resize(this->data()->size() + append_size);
        }

        std::size_t cur_indices_offset_bytes = this->indices()->size();

        {
            std::size_t append_size = data.indices_size_bytes();
            this->indices()->safe_resize(this->indices()->size() + append_size);
        }

        // Now insert the data...
        {
            void* data_begin = data.vertices.data();
            tz::mem::Block data_blk{data_begin, data.data_size_bytes()};
            this->data()->send(cur_offset_bytes, data_blk);
        }

        // And now the indices...
        {
            void* indices_begin = data.indices.data();
            tz::mem::Block indices_blk{indices_begin, data.indices_size_bytes()};
            this->indices()->send(cur_indices_offset_bytes, indices_blk);
        }

        // Make sure we start tracking this properly.
        MeshInfo info{cur_offset_vertices, mesh_size_vertices};
        Handle handle = this->mesh_info_map.size();
        this->mesh_info_map.emplace(handle, info);

        return handle;
    }

    std::size_t Manager::get_indices_offset(Handle handle) const
    {
        topaz_assert(this->mesh_info_map.count(handle) == 1, "tz::gl::Manager::get_indices_offset(Handle): This Manager has no knowledge of this handle ", handle, ". Cannot retrieve information about this handle...");
        return this->mesh_info_map.at(handle).offset_vertices;
    }

    std::size_t Manager::get_number_of_vertices(Handle handle) const
    {
        topaz_assert(this->mesh_info_map.count(handle) == 1, "tz::gl::Manager::get_number_of_vertices(Handle): This Manager has no knowledge of this handle ", handle, ". Cannot retrieve information about this handle...");
        return this->mesh_info_map.at(handle).size_vertices;
    }

    tz::gl::Object& Manager::operator*()
    {
        return this->o;
    }

    const tz::gl::Object& Manager::operator*() const
    {
        return this->o;
    }

    std::size_t Manager::get_indices() const
    {
        return this->index_handle;
    }

    tz::gl::VBO* Manager::data()
    {
        return this->o.get<tz::gl::BufferType::Array>(this->data_handle);
    }

    const tz::gl::VBO* Manager::data() const
    {
        return this->o.get<tz::gl::BufferType::Array>(this->data_handle);
    }

    tz::gl::IBO* Manager::indices()
    {
        return this->o.get<tz::gl::BufferType::Index>(this->index_handle);
    }

    const tz::gl::IBO* Manager::indices() const
    {
        return this->o.get<tz::gl::BufferType::Index>(this->index_handle);
    }
}