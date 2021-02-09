#ifndef TOPAZ_RENDER_ASSET_HPP
#define TOPAZ_RENDER_ASSET_HPP
#include "core/containers/unique_vector.hpp"
#include "gl/manager.hpp"
#include "core/bool.hpp"
#include "render/device.hpp"
#include <optional>

namespace tz::render
{
    /**
	 * \addtogroup tz_render Topaz Rendering Library (tz::render)
	 * High-level interface for 3D and 2D hardware-accelerated graphics programming. Used in combination with the \ref tz_gl "Topaz Graphics Library".
	 * @{
	 */
    /**
     * Represents existing mesh data that is already stored within a gl::Manager.
     */
    class MeshAsset
    {
    public:
        /**
         * Construct a MeshAsset based upon existing mesh data within the given gl::Manager handle.
         * @param manager Manager owning the mesh data.
         * @param handle Handle representing the mesh data region within the gl::Manager's data-store.
         */
        MeshAsset(tz::gl::Manager& manager, tz::gl::Manager::Handle handle);
        /// Shallow-comparison.
        bool operator==(const MeshAsset& rhs) const;
        /**
         * Invoke the MeshAsset, emitting an IndexSnippet. This can be given to an IndexSnippetList, so long as its device correctly references the same gl::Object.
         * @return An IndexSnippet, which locates the mesh data in gl::VBO memory.
         */
        tz::gl::IndexSnippet operator()() const;
        /**
         * Query as to whether this asset shares the given gl::Object and IBO handle. If any of them don't match, they will be assigned to this MeshAsset's Object/IBO.
         * Note: It is not an error to pass nullptr/an invalid IBO handle in this case. In this case they will simply be assigned to match this asset.
         * @param object Reference to the object pointer to compare.
         * @param ibo Reference to the IBO handle to compare.
         * @return True if the object pointer & IBOs match and no assignment took place. Otherwise false.
         */
        bool shares(tz::gl::Object*& object, std::size_t& ibo) const;
        /**
         * Retrieve the owning manager.
         * @return Reference to the manager owning this mesh asset.
         */
        const tz::gl::Manager& get_manager() const;
    private:
        tz::gl::Manager* manager;
        tz::gl::Manager::Handle handle;
    };

    /**
     * Stores a collection of MeshAssets.
     */
    class AssetBuffer
    {
    public:
        using Index = std::size_t;
        static constexpr bool enable_mesh_by_default = true;

        /// Construct an empty AssetBuffer.
        AssetBuffer() = default;
        /**
         * Add a MeshAsset to the buffer.
         * Precondition: This mesh must share a common gl::Object source and IBO handle with all existing meshes within the buffer. Otherwise, this will assert.
         * @param mesh Mesh to add to the buffer.
         * @return Index referring to the mesh.
         */
        Index add_mesh(MeshAsset mesh);
        /**
         * Apply the common gl::Object and IBO handle to the given device.
         * Note: It does not attempt to render with the device.
         * @param device Device to configure for any of these meshes.
         */
        void apply(tz::render::Device& device) const;
        /**
         * Retrieve the Mesh at the given index.
         * Precondition: A MeshAsset must exist at the given index. Otherwise, this will assert and invoke UB.
         * Note: It is wrong to cache this reference. The reference may be invalidated at anytime, but the index will remain valid until the mesh is erased.
         * @param idx Index of the mesh to retrieve.
         * @return Const reference to the MeshAsset at the given index.
         */
        const tz::render::MeshAsset& at(Index idx) const;
        /**
         * Retrieve the Mesh at the given index.
         * Precondition: A MeshAsset must exist at the given index. Otherwise, this will assert and invoke UB.
         * Note: It is wrong to cache this reference. The reference may be invalidated at anytime, but the index will remain valid until the mesh is erased.
         * @param idx Index of the mesh to retrieve.
         * @return Reference to the MeshAsset at the given index.
         */
        tz::render::MeshAsset& at(Index idx);
        /**
         * Retrieve the common manager of all meshes within the buffer.
         * @return Common manager.
         */
        const tz::gl::Manager& get_manager() const;
    private:
        tz::UniqueVector<MeshAsset> meshes;
        std::size_t common_ibo_handle = std::numeric_limits<std::size_t>::max();
        tz::gl::Object* common_object = nullptr;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_RENDER_ASSET_HPP