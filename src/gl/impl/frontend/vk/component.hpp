#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_COMPONENT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_COMPONENT_HPP
#if TZ_VULKAN
#include "gl/api/component.hpp"
#include "gl/impl/backend/vk/buffer.hpp"

namespace tz::gl
{
    class BufferComponentVulkan : public IComponent
    {
    public:
        BufferComponentVulkan(IResource* resource);
        BufferComponentVulkan() = default;
        const IResource* get_resource() const override;
        IResource* get_resource() override;
        const vk::Buffer& get_buffer() const;
        vk::Buffer& get_buffer();
        void set_buffer(vk::Buffer buffer);
    private:
        vk::Buffer buffer = vk::Buffer::null();
        IResource* resource = nullptr;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_COMPONENT_HPP