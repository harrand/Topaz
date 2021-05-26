#ifndef TOPAZ_GL_API_RESOURCE_HPP
#define TOPAZ_GL_API_RESOURCE_HPP
#include "gl/impl/common/renderer.hpp"
#include <memory>
#include <span>
#include <cstdint>

namespace tz::gl
{
    class IResource
    {
        [[nodiscard]] virtual std::unique_ptr<IResource> unique_clone() const = 0;

        virtual RendererDataAccess data_access() const{return RendererDataAccess::StaticFixed;}
        virtual std::span<const std::byte> get_resource_data() const = 0;
    };

    template<class Derived>
    class IResourceCopyable : public IResource
    {
        [[nodiscard]] virtual std::unique_ptr<IResource> unique_clone() const final
        {
            static_assert(requires{requires std::copyable<Derived>;}, "IResourceCopyable<T>: T must be copyable. Derive from IResource and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };
}

#endif // TOPAZ_GL_API_RESOURCE_HPP