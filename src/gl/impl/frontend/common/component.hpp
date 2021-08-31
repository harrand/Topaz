#ifndef TOPAZ_GL_IMPL_FRONTEND_COMMON_COMPONENT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_COMMON_COMPONENT_HPP
#include "gl/api/component.hpp"
#include "gl/api/resource.hpp"
#include <memory>
#include <vector>

namespace tz::gl
{
    class ComponentManager
    {
    public:
        ComponentManager() = default;
        ResourceHandle add_component(IComponent* component);
        template<typename ComponentType, typename... Ts>
        ResourceHandle emplace_component(Ts&&... args);

        const IComponent* get_component(ResourceHandle handle) const;
        IComponent* get_component(ResourceHandle handle);
        std::span<const IComponent* const> components(ResourceType type) const;
        std::span<IComponent* const> components(ResourceType type);

        std::size_t component_count() const;
        std::size_t component_count_of(ResourceType type) const;
    private:
        std::size_t total_components() const;
        
        std::vector<std::unique_ptr<IComponent>> owned_buffer_components;
        std::vector<std::unique_ptr<IComponent>> owned_texture_components;
        std::vector<IComponent*> buffer_components;
        std::vector<IComponent*> texture_components;
    };
}
#include "gl/impl/frontend/common/component.inl"
#endif // TOPAZ_GL_IMPL_FRONTEND_COMMON_COMPONENT_HPP