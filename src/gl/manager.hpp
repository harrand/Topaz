#ifndef TOPAZ_GL_MANAGER_HPP
#define TOPAZ_GL_MANAGER_HPP
#include "gl/object.hpp"
#include "gl/mesh.hpp"

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

    struct StandardDataRegionNames
    {
        std::string positions_name;
        std::string texcoords_name;
        std::string normals_name;
        std::string tangents_name;
        std::string bi_tangents_name;
    };

    class Manager
    {
    public:
        Manager();
        void add_data(Data type, const MeshData& data, const StandardDataRegionNames& names);
        tz::mem::OwningBlock get_data(Data type, const std::string& region_name);
    private:
        void init();

        tz::gl::Object o;

        tz::gl::ManagedVBO* static_data;
        tz::gl::ManagedTVBO* dynamic_data;
        tz::gl::ManagedIBO* static_indices;
        tz::gl::ManagedTIBO* dynamic_indices;
    };
}

#endif // TOPAZ_GL_MANAGER_HPP