//
// Created by Harrand on 08/02/2020.
//

#ifndef TOPAZ_GL_MANAGER_HPP
#define TOPAZ_GL_MANAGER_HPP
#include "gl/object.hpp"
#include "gl/mesh.hpp"
#include <unordered_map>

namespace tz::gl
{
    /**
     * Managed tz::gl::Object which implicitly has data buffers prepared. This should be used in-place of a tz::gl::Object if no special data format is required and normal meshes are to be used.
     * Managers don't create any terminal buffers by default. If you want them, it is recommended to retrieve the manager's object and create them yourself. The manager will never attempt to use any buffers that it hasn't created itself.
     * Contains:
     * - Data Buffer
     * - Indices Buffer
     */
    class Manager
    {
    public:
        using Handle = std::size_t;

        /**
         * Construct an empty Manager.
         */
        Manager();
        
        Handle add_mesh(tz::gl::IndexedMesh data);
        std::size_t get_vertices_offset(Handle handle) const;
        std::size_t get_number_of_vertices(Handle handle) const;
        Handle partition(Handle handle, std::size_t vertex_offset);
        std::vector<Handle> split(Handle handle, std::size_t stride_vertices);
    
        tz::gl::Object& operator*();
        const tz::gl::Object& operator*() const;
        std::size_t get_indices() const;
    private:
        tz::gl::VBO* data();
        const tz::gl::VBO* data() const;
        tz::gl::IBO* indices();
        const tz::gl::IBO* indices() const;

        struct MeshInfo
        {
            std::size_t offset_vertices;
            std::size_t size_vertices;
        };

        tz::gl::Object o;
        
        std::size_t data_handle;
        std::size_t index_handle;
        std::unordered_map<Handle, MeshInfo> mesh_info_map;
    };
}

#endif // TOPAZ_GL_MANAGER_HPP