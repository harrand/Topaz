#ifndef TOPAZ_GL_API_RESOURCE_HPP
#define TOPAZ_GL_API_RESOURCE_HPP
#include "core/interfaces/cloneable.hpp"
#include "core/handle.hpp"
#include "gl/impl/common/resource.hpp"
#include "gl/impl/common/renderer.hpp"
#include <span>

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
    class IResource : public tz::IUniqueCloneable<IResource>
    {
    public:
        virtual constexpr ResourceType get_type() const = 0;
        virtual constexpr RendererInputDataAccess data_access() const {return RendererInputDataAccess::StaticFixed;}
        virtual std::span<const std::byte> get_resource_bytes() const = 0;
    };

    using ResourceHandle = tz::Handle<IResource>;

    template<typename Derived>
    class IResourceCloneable : public IResource
    {
    public:
        /// Invokes `Derived::Derived(const Derived&)`
        [[nodiscard]] virtual std::unique_ptr<IResource> unique_clone() const final
        {
            static_assert(requires{requires std::copyable<Derived>;}, "IResourceCopyable<T>: T must be copyable. Derive from IResource and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };

    class IDynamicResource : public IResource
    {
    public:
        virtual constexpr RendererInputDataAccess data_access() const final{return RendererInputDataAccess::DynamicFixed;}
        virtual std::span<std::byte> get_resource_bytes_dynamic() = 0;

        #if TZ_VULKAN
            friend class RendererBufferManagerVulkan;
        #elif TZ_OGL
            friend class RendererOGL;
        #endif
    private:
        virtual void set_resource_data(std::byte* resource_data) = 0;
    };

    template<typename Derived>
    class IDynamicResourceCloneable : public IDynamicResource
    {
    public:
        /// Invokes `Derived::Derived(const Derived&)`
        [[nodiscard]] virtual std::unique_ptr<IResource> unique_clone() const final
        {
            static_assert(requires{requires std::copyable<Derived>;}, "IDynamicResourceCopyable<T>: T must be copyable. Derive from IDynamicResource and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };

    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_RESOURCE_HPP