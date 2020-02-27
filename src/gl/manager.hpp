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
        /**
         * Copy the data of an indexed mesh into the Manager's internal buffers and retrieve a handle which can be used to ascertain the location of the data.
         * @param data Indexed mesh data to copy into the internal buffers.
         * @return Opaque handle corresponding to the copied mesh data.
         */
        Handle add_mesh(tz::gl::IndexedMesh data);
        /**
         * Retrieve the number of vertices preceding the first vertex corresponding to the indexed mesh data associated with the given handle.
         * @param handle Handle whose mesh data offset should be retrieved.
         * @return Number of vertices between the beginning of the internal buffer's data, and the beginning of the handle's mesh data.
         */
        std::size_t get_vertices_offset(Handle handle) const;
        std::size_t get_indices_offset(Handle handle) const;
        /**
         * Retrieve the number of vertices corresponding to the indexed mesh data associated with the given handle.
         * @param handle Handle whose mesh data size should be retrieved.
         * @return Number of vertices comprising the mesh data.
         */
        std::size_t get_number_of_vertices(Handle handle) const;
        std::size_t get_number_of_indices(Handle handle) const;
        /**
         * Relinquish a handle's ownership of some or all of its internal mesh data.
         * 
         * Precondition: The given handle has previously been created by this Manager. Otherwise, this will assert and invoke UB.
         * Precondition: The vertex offset provided is smaller than the number of vertices occupied by the handle. Otherwise, this will assert and invoke UB.
         * Example Scenario:
         * [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
         * |----------------------------|
         *             Handle 0
         * manager->partition(0, 3);
         * New data arrangement:
         * [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
         * |------|  |-----------------|
         * Handle 0         Handle X
         * (Returns X)
         * 
         * @param handle Handle corresponding to the initial mesh data.
         * @param vertex_offset Desired number of vertices corresponding to the given handle. The remainder of these vertices will be owned by the new handle.
         * @return Handle corresponding to the remainder of the vertices taken away from the initial Handle.
         */
        Handle partition(Handle handle, std::size_t vertex_offset);
        /**
         * Split the internal mesh data corresponding to a given handle into a set of daughter handles. Each daughter handle is expected to own the same amount of vertices.
         * This is useful to split a large mesh into multiple sub-meshes.
         * Note: This is not tessellation or mesh-cutting -- No mesh data is actually manipulated at all.
         * Note: The daughter handles will also contain the initial handle. This is guaranteed to be its first element.
         * Precondition: Number of vertices occupied by the given handle is divisible by the stride. This is necessary to ensure that all daughters have the same vertex quantity. If not, this will assert and invoke UB.
         * Precondition: Stride must be greater than 0. Otherwise, this will invoke UB without asserting.
         * 
         * Example Scenario:
         * [0, 1, 2, 3, 4, 5]
         * |----------------|
         *      Handle 0
         * manager->split(0, 2);
         * New data arrangement:
         * [0, 1, 2, 3, 4, 5]
         *  |--|  |--|  |--|
         *   h0    hX    hY
         * (Returns {0, X, Y})
         */
        std::vector<Handle> split(Handle handle, std::size_t stride_vertices);
        /**
         * Retrieve the underlying tz::gl::Object. There are few functional restrictions to using the object this way.
         * This is a very unsafe API method. This is exposed only for potential optimisations and ease-of-use in demos. It is recommended not to use this API method as it is likely to become deprecated in the future.
         * There are some recommendations:
         * 
         * - While it is not an error to read-from or write-to any of the existing buffers, know that these buffers are used by this Manager.
         *      - Resizing these buffers however, will invoke UB without asserting.
         *      - It is also okay to map these buffers. However, they must be unmapped prior to the next invocation of this->add_mesh(). Otherwise, this will invoke UB without asserting.
         * - It is well-defined to make these existing buffers terminal. However, you must not use this->add_mesh() to add any additional mesh data. If you do this when the buffers are terminal, this will invoke UB without asserting.
         * - Erasing any existing buffers of this Object which you did not create yourself will invoke UB without asserting.
         */
        tz::gl::Object& operator*();
        /**
         * Retrieve the underlying tz::gl::Object. There are few functional restrictions to using the object this way.
         * This is a very unsafe API method. This is exposed only for potential optimisations and ease-of-use in demos. It is recommended not to use this API method as it is likely to become deprecated in the future.
         * There are some recommendations:
         * 
         * - While it is not an error to read-from or write-to any of the existing buffers, know that these buffers are used by this Manager.
         *      - Resizing these buffers however, will invoke UB without asserting.
         *      - It is also okay to map these buffers. However, they must be unmapped prior to the next invocation of this->add_mesh(). Otherwise, this will invoke UB without asserting.
         * - It is well-defined to make these existing buffers terminal. However, you must not use this->add_mesh() to add any additional mesh data. If you do this when the buffers are terminal, this will invoke UB without asserting.
         * - Erasing any existing buffers of this Object which you did not create yourself will invoke UB without asserting.
         */
        const tz::gl::Object& operator*() const;
        /**
         * Retrieve a handle corresponding to the internal index-buffer handle used by this Manager.
         * 
         * Note: This is a very unsafe API method. This is exposed only for potential optimisations and ease-of-use in demos. It is recommended not to use this API method as it is likely to become deprecated in the future.
         */
        std::size_t get_indices() const;
    private:
        tz::gl::VBO* data();
        const tz::gl::VBO* data() const;
        tz::gl::IBO* indices();
        const tz::gl::IBO* indices() const;

        struct MeshInfo
        {
            std::size_t offset_vertices;
            std::size_t offset_indices;
            std::size_t size_vertices;
            std::size_t size_indices;
        };

        tz::gl::Object o;
        
        std::size_t data_handle;
        std::size_t index_handle;
        std::unordered_map<Handle, MeshInfo> mesh_info_map;
    };
}

#endif // TOPAZ_GL_MANAGER_HPP