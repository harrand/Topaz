#if TZ_VULKAN
#include "gl/impl/frontend/vk/component.hpp"
#include <utility>

namespace tz::gl
{
    BufferComponentVulkan::BufferComponentVulkan(IResource* resource):
    buffer(vk::Buffer::null()),
    resource(resource)
    {

    }

    const IResource* BufferComponentVulkan::get_resource() const
    {
        return this->resource;
    }

    IResource* BufferComponentVulkan::get_resource()
    {
        return this->resource;
    }

    const vk::Buffer& BufferComponentVulkan::get_buffer() const
    {
        return this->buffer;
    }

    vk::Buffer& BufferComponentVulkan::get_buffer()
    {
        return this->buffer;
    }

    void BufferComponentVulkan::set_buffer(vk::Buffer buffer)
    {
        this->buffer = std::move(buffer);
    }

    TextureComponentVulkan::TextureComponentVulkan(IResource* resource, vk::Image img, vk::ImageView view, vk::Sampler sampler):
    resource(resource),
    img(std::move(img)),
    view(std::move(view)),
    sampler(std::move(sampler))
    {}

    TextureComponentVulkan::TextureComponentVulkan(vk::Image img, vk::ImageView view, vk::Sampler sampler):
    TextureComponentVulkan(nullptr, std::move(img), std::move(view), std::move(sampler)){}

    void TextureComponentVulkan::clear_and_resize(unsigned int width, unsigned int height)
    {
        const vk::LogicalDevice& dev = this->img.get_device();
        vk::Image::Format fmt = this->img.get_format();
        vk::Image::Usage usage = this->img.get_usage();
        vk::hardware::MemoryResidency residency = this->img.get_memory_residency();
        this->img = vk::Image(dev, width, height, fmt, vk::Image::UsageField{usage}, residency);
        this->view = vk::ImageView(dev, this->img);
        this->sampler = vk::Sampler(dev, this->sampler.get_properties());
    }


    const IResource* TextureComponentVulkan::get_resource() const
    {
        return this->resource;
    }

    IResource* TextureComponentVulkan::get_resource()
    {
        return this->resource;
    }

    const vk::Image& TextureComponentVulkan::get_image() const
    {
        return this->img;
    }

    const vk::ImageView& TextureComponentVulkan::get_view() const
    {
        return this->view;
    }

    const vk::Sampler& TextureComponentVulkan::get_sampler() const
    {
        return this->sampler;
    }
}

#endif // TZ_VULKAN