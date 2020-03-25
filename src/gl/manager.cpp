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
        std::size_t indices_offset_bytes = this->indices()->size();
        std::size_t indices_offset_indices = indices_offset_bytes / sizeof(unsigned int);
        std::size_t mesh_size_vertices, mesh_size_indices;
        {
            std::size_t append_size = data.data_size_bytes();
            mesh_size_vertices = append_size / sizeof(tz::gl::Vertex);
            mesh_size_indices = data.indices_size_bytes() / sizeof(unsigned int);
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
        MeshInfo info{cur_offset_vertices, indices_offset_indices, mesh_size_vertices, mesh_size_indices};
        Handle handle = this->mesh_info_map.size();
        this->mesh_info_map.emplace(handle, info);

        return handle;
    }

    std::size_t Manager::get_vertices_offset(Handle handle) const
    {
        topaz_assert(this->mesh_info_map.count(handle) == 1, "tz::gl::Manager::get_vertices_offset(Handle): This Manager has no knowledge of this handle ", handle, ". Cannot retrieve information about this handle...");
        return this->mesh_info_map.at(handle).offset_vertices;
    }

    std::size_t Manager::get_indices_offset(Handle handle) const
    {
        topaz_assert(this->mesh_info_map.count(handle) == 1, "tz::gl::Manager::get_indices_offset(Handle): This Manager has no knowledge of this handle ", handle, ". Cannot retrieve information about this handle...");
        return this->mesh_info_map.at(handle).offset_indices;
    }

    std::size_t Manager::get_number_of_vertices(Handle handle) const
    {
        topaz_assert(this->mesh_info_map.count(handle) == 1, "tz::gl::Manager::get_number_of_vertices(Handle): This Manager has no knowledge of this handle ", handle, ". Cannot retrieve information about this handle...");
        return this->mesh_info_map.at(handle).size_vertices;
    }

    std::size_t Manager::get_number_of_indices(Handle handle) const
    {
        topaz_assert(this->mesh_info_map.count(handle) == 1, "tz::gl::Manager::get_number_of_indices(Handle): This Manager has no knowledge of this handle ", handle, ". Cannot retrieve information about this handle...");
        return this->mesh_info_map.at(handle).size_indices;
    }

    typename Manager::Handle Manager::partition(Handle handle, std::size_t vertex_offset)
    {
        // We require the given handle to already be managed.
        topaz_assert(this->mesh_info_map.find(handle) != this->mesh_info_map.end(), "tz::gl::Manager::partition(", handle, ", ", vertex_offset, "): Manager does not know about handle '", handle, "' -- So cannot partition!");
        MeshInfo& info = this->mesh_info_map[handle];
        std::size_t original_vertices_size = info.size_vertices;
        std::size_t original_indices_size = info.size_indices;
        // Ensure that the byte offset is less than our size.
        topaz_assert(info.size_vertices > vertex_offset, "tz::gl::Manager::partition(", handle, ", ", vertex_offset, "): The given handle '", handle, "' cannot be partitioned at offset ", vertex_offset, " because this handle only occupies ", info.size_vertices, " vertices");
        // Essentially we set our size to be equal to the offset, so that the new handle can manage the remainder of the vertices.
        info.size_vertices = vertex_offset;
        info.size_indices = vertex_offset;
        Handle new_handle = this->mesh_info_map.size();
        std::size_t new_offset_vertices = info.offset_vertices + info.size_vertices;
        std::size_t new_offset_indices = info.offset_indices + info.size_indices;
        // All the vertices which the first handle no longer owns, we will take.
        std::size_t new_vertices_size = original_vertices_size - info.size_vertices;
        std::size_t new_indices_size = original_indices_size - info.size_indices;
        MeshInfo new_info{new_offset_vertices, new_offset_indices, new_vertices_size, new_indices_size};
        this->mesh_info_map.emplace(new_handle, new_info);
        return new_handle;
    }

    std::vector<typename Manager::Handle> Manager::split(Handle handle, std::size_t stride_vertices)
    {
        // Firstly, make sure the stride we get fits properly
        std::size_t initial_vertex_count = this->get_number_of_vertices(handle);
        topaz_assert(initial_vertex_count % stride_vertices == 0, "tz::gl::Manager::split(", handle, ", ", stride_vertices, "): Splitting by stride ", stride_vertices, " doesn't make sense as the number of vertices contained within handle ", handle, " is ", initial_vertex_count, ", which is not divisible by ", stride_vertices);
        std::size_t split_amount = initial_vertex_count / stride_vertices;
        // Create the underlying container.
        std::vector<typename Manager::Handle> daughter_handles;
        daughter_handles.reserve(split_amount);
        daughter_handles.push_back(handle);
        // Partition the main handle until we have an equal split in all.
        for(std::size_t i = 0; i < split_amount - 1; i++)
        {
            daughter_handles.push_back(this->partition(daughter_handles.back(), stride_vertices));
        }
        return std::move(daughter_handles);
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