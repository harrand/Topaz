#include "render/asset.hpp"
#include "algo/container.hpp"

namespace tz::render
{
    MeshAsset::MeshAsset(tz::gl::Manager& manager, tz::gl::Manager::Handle handle): manager(&manager), handle(handle){}

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

    const tz::gl::Manager& MeshAsset::get_manager() const
    {
        return *this->manager;
    }

    AssetBuffer::Index AssetBuffer::add_mesh(MeshAsset mesh)
    {
            Index size = this->meshes.size();
            bool shares = mesh.shares(this->common_object, this->common_ibo_handle);
            topaz_assert(shares || size == 0, "AssetBuffer elements do not all share a common gl::Object and IBO handle.");
            bool insertion_successful = this->meshes.push_back(mesh);
            if(insertion_successful)
            {
                return size;
            }
            else
            {
                Index idx = std::distance(this->meshes.begin(), std::find(this->meshes.begin(), this->meshes.end(), mesh));
                return idx;
            }
    }

    void AssetBuffer::apply(tz::render::Device& device) const
    {
        // Populate the list with snippets. If the mesh mask for the given bit is disabled, then its snippet will be skipped.
        device.set_object(this->common_object);
        device.set_handle(this->common_ibo_handle);
    }

    const tz::render::MeshAsset& AssetBuffer::at(Index idx) const
    {
        topaz_assert(this->meshes.size() > idx, "AssetBuffer::at(", idx, "): Out of range. Size = ", this->meshes.size());
        return this->meshes[idx];
    }

    tz::render::MeshAsset& AssetBuffer::at(Index idx)
    {
        topaz_assert(this->meshes.size() > idx, "AssetBuffer::at(", idx, "): Out of range. Size = ", this->meshes.size());
        return this->meshes[idx];
    }

    const tz::gl::Manager& AssetBuffer::get_manager() const
    {
        topaz_assert(!this->meshes.empty(), "AssetBuffer::get_manager(): No meshes stored; no common manager exists.");
        return this->meshes[0].get_manager();
    }
}