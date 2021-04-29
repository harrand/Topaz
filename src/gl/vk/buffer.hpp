#ifndef TOPAZ_GL_VK_BUFFER_HPP
#define TOPAZ_GL_VK_BUFFER_HPP
#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include <cstdint>

namespace tz::gl::vk
{
    enum class BufferType
    {
        Vertex
    };

    class Buffer
    {
    public:
        Buffer(BufferType type, const LogicalDevice& device, std::size_t size_bytes);
        Buffer(const Buffer& copy) = delete;
        Buffer(Buffer&& move);
        ~Buffer();

        void write(const void* addr, std::size_t bytes);

        Buffer& operator=(const Buffer& rhs) = delete;
        Buffer& operator=(Buffer&& rhs);

        VkBuffer native() const;
    private:
        VkBuffer buffer;
        VkDeviceMemory memory;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif //TOPAZ_GL_VK_BUFFER_HPP