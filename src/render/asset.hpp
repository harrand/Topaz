#ifndef TOPAZ_RENDER_ASSET_HPP
#define TOPAZ_RENDER_ASSET_HPP
#include "gl/manager.hpp"
#include "geo/bool.hpp"
#include "render/device.hpp"
#include <optional>

namespace tz::render
{
    class MeshAsset
    {
    public:
        MeshAsset(tz::gl::Manager* manager, tz::gl::Manager::Handle handle);
        bool operator==(const MeshAsset& rhs) const;
        tz::gl::IndexSnippet operator()() const;
        bool shares(tz::gl::Object*& object, std::size_t& ibo) const;
    private:
        tz::gl::Manager* manager;
        tz::gl::Manager::Handle handle;
    };

    class AssetBuffer
    {
    public:
        using Handle = std::size_t;
        static constexpr bool enable_mesh_by_default = true;

        AssetBuffer() = default;
        Handle add_mesh(MeshAsset mesh);
        void apply(tz::render::Device& device) const;
        const tz::render::MeshAsset& at(Handle handle) const;
    private:
        std::vector<MeshAsset> meshes;
        std::vector<tz::geo::BoolProxy> mesh_mask;
        std::size_t common_ibo_handle = std::numeric_limits<std::size_t>::max();
        tz::gl::Object* common_object = nullptr;
    };
}

#endif // TOPAZ_RENDER_ASSET_HPP