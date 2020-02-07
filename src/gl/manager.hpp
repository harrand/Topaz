#ifndef TOPAZ_GL_MANAGER_HPP
#define TOPAZ_GL_MANAGER_HPP
#include "gl/object.hpp"
#include "gl/mesh_data.hpp"

namespace tz::gl
{
    enum class Data
    {
        Static,
        Dynamic
    };

    enum class Indices
    {
        Static,
        Dynamic
    };

    /**
     * Wrapper around a tz::gl::Object using pre-defined managed buffers.
     * Consists of:
     * - Managed Nonterminal VBO (Data::Static)
     * - Managed Terminal VBO (Data::Dynamic)
     * - Managed Nonterminal IBO (Indices::Static)
     * - Managed Terminal IBO (Indices::Dynamic)
     * 
     * Note: Underlying managed terminal buffers are not yet implemented.
     */
    class Manager
    {
    public:
        Manager();
        /**
         * Add mesh data to a managed VBO, creating regions for the following data types:
         * - Position
         * - Texture Coordinate
         * - Normal
         * - Tangent
         * - Bi-tangent
         * @param type Description of how often the data is expected to change. Dynamic data will be stored in a separate terminal VBO, but static will not.
         * @param data Mesh data structure containing all data to manage. Data will be instantly copied into the underlying buffers.
         * @param names Region name structure containing region names for the various data types. This makes it easy to define attributes for such types.
         */
        void add_data(Data type, const MeshData& data, const StandardDataRegionNames& names);
        /**
         * Add indices data to a managed IBO, creating a region for the new indices.
         * TODO: Auto-increment the indices such that they all reference the newest elements in the managed VBO.
         * @param type Description of how often the indices are expected to change. Dynamic indices will be stored in a separate terminal IBO, but static will not.
         * @param indices Data stucture containing all the indices to manage. Indices will be instantly copied into the underlying buffers.
         * @param region_name Region name comprising the new area of the buffer where these new indices are stored.
         */
        void add_indices(Indices type, const MeshIndices& indices, std::string region_name);
        /**
         * Adds both mesh and index data to the underlying VBOs/IBOs.
         * This is equivalent to invoking add_data(dtype, mesh.data, mesh.data_names) followed by add_indices(itype, mesh.indices, mesh.indices_names).
         * @param dtype Description of how the mesh data is expected to change. Dynamic data will be put into a separate terminal buffer.
         * @param itype Description of how the mesh indices are expected to change. Dynamic indices will be put into a separate terminal buffer.
         * @param mesh Mesh containing vertex data and indices.
         */
        void add_mesh(Data dtype, Indices itype, const Mesh& mesh);
        /**
         * Retrieve an owning memory block containing a copy of the existing region with the given name.
         * @param type Description of which type of VBO to find the existing data. Dynamic will search the terminal VBO, Static will search the nonterminal.
         * @param region_name Name of the region whose data is of interest.
         * @return Owning memory-block containing a copy of the region's underlying data.
         */
        tz::mem::OwningBlock get_data(Data type, const std::string& region_name);
        /**
         * Retrieve an owning memory block containing a copy of the existing region with the given name.
         * @param type Description of which type of IBO to find the existing data. Dynamic will search the terminal IBO, Static will search the nonterminal.
         * @param region_name Name of the region whose indices are of interest.
         * @return Owning memory-block containing a copy of the region's underlying index-data.
         */
        tz::mem::OwningBlock get_indices(Indices type, const std::string& region_name);
        /**
         * Define a vertex attribute starting at the first element of the region with the given name.
         * @param type Description of which type of VBO to find the region. Dynamic will search the terminal VBO, Static will search the nonterminal.
         * @param region_name Name of the region whose beginning byte offset the attribute begins.
         * @param fmt The format of the attribute. Note that the offset component of this format is unused (The offset that the region starts at will be used instead).
         */
        void attrib(Data type, std::string region_name, tz::gl::Format fmt);
        tz::gl::Object* operator->();
        const tz::gl::Object* operator->() const;
        tz::gl::Object& operator*();
        const tz::gl::Object& operator*() const;
    private:
        void init();
        tz::gl::ManagedVBO* static_data();
        tz::gl::ManagedTVBO* dynamic_data();
        tz::gl::ManagedIBO* static_indices();
        tz::gl::ManagedTIBO* dynamic_indices();

        tz::gl::Object o;

        std::size_t static_data_id;
        std::size_t dynamic_data_id;
        std::size_t static_indices_id;
        std::size_t dynamic_indices_id;
    };
}

#endif // TOPAZ_GL_MANAGER_HPP