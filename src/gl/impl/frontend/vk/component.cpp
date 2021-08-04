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
}

#endif // TZ_VULKAN