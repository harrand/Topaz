#include "render/asset.hpp"
#include "algo/container.hpp"

namespace tz::render
{
    MeshAsset::MeshAsset(tz::gl::Manager* manager, tz::gl::Manager::Handle handle): manager(manager), handle(handle){}

    bool MeshAsset::operator==(const MeshAsset& rhs) const
    {
        return this->manager == rhs.manager && this->handle == rhs.handle;
    }

    tz::gl::IndexSnippet MeshAsset::operator()() const
    {
        return {*this->manager, this->handle};
    }

    bool MeshAsset::shares(tz::gl::Object*& object, std::size_t& ibo) const
    {
        tz::gl::Object& mng_obj = **this->manager;
        bool obj_same = (object != nullptr && (mng_obj == *object));
        if(!obj_same)
        {
            object = &mng_obj;
        }
        std::size_t mng_ibo = this->manager->get_indices();
        bool ibo_same = (mng_ibo == ibo);
        if(!ibo_same)
        {
            ibo = mng_ibo;
        }
        return obj_same && ibo_same;
    }

    AssetBuffer::Handle AssetBuffer::add_mesh(MeshAsset mesh)
    {
            std::size_t size = this->meshes.size();
            this->meshes.push_back(mesh);
            MeshAsset& new_mesh = this->meshes.back();
            this->mesh_mask.push_back(enable_mesh_by_default);
            if(!new_mesh.shares(this->common_object, this->common_ibo_handle) && size > 0)
            {
                topaz_assert(false, "AssetBuffer elements do not all share a common gl::Object and IBO handle.");
            }
            return size;
    }

    void AssetBuffer::apply(tz::render::Device& device) const
    {
        #if TOPAZ_DEBUG
        auto a = this->meshes.size();
        auto b = this->mesh_mask.size();
        topaz_assert(a == b, "tz::render::AssetBuffer::apply(Device): AssetBuffer malformed because it stores ", a, " meshes but the mask only contains ", b, " elements.");
        #endif
        // Populate the list with snippets. If the mesh mask for the given bit is disabled, then its snippet will be skipped.
        device.set_object(this->common_object);
        device.set_handle(this->common_ibo_handle);
    }

    const tz::render::MeshAsset& AssetBuffer::at(Handle handle) const
    {
        return this->meshes[handle];
    }
}