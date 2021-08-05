#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_COMPONENT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_COMPONENT_HPP
#if TZ_VULKAN
#include "gl/api/component.hpp"

#include "gl/impl/backend/vk/buffer.hpp"

#include "gl/impl/backend/vk/image.hpp"
#include "gl/impl/backend/vk/image_view.hpp"
#include "gl/impl/backend/vk/sampler.hpp"

namespace tz::gl
{
    class BufferComponentVulkan : public IComponent
    {
    public:
        BufferComponentVulkan(IResource* resource);
        BufferComponentVulkan() = default;
        virtual const IResource* get_resource() const override;
        virtual IResource* get_resource() override;
        const vk::Buffer& get_buffer() const;
        vk::Buffer& get_buffer();
        void set_buffer(vk::Buffer buffer);
    private:
        vk::Buffer buffer = vk::Buffer::null();
        IResource* resource = nullptr;
    };

    class TextureComponentVulkan : public IComponent
    {
    public:
        TextureComponentVulkan(IResource* resource, vk::Image img, vk::ImageView view, vk::Sampler sampler);
        TextureComponentVulkan(vk::Image img, vk::ImageView view, vk::Sampler sampler);
        void clear_and_resize(unsigned int width, unsigned int height);
        virtual const IResource* get_resource() const override;
        virtual IResource* get_resource() override;
        const vk::Image& get_image() const;
        const vk::ImageView& get_view() const;
        const vk::Sampler& get_sampler() const;
    private:
        IResource* resource;
        vk::Image img;
        vk::ImageView view;
        vk::Sampler sampler;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_COMPONENT_HPP