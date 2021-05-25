#ifndef TOPAZ_GL_VK_BUFFER_HPP
#define TOPAZ_GL_VK_BUFFER_HPP
#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include <cstdint>

namespace tz::gl::vk
{
    enum class BufferType
    {
        Vertex,
        Index,
        Staging,
        Uniform
    };

    enum class BufferPurpose
    {
        TransferSource,
        TransferDestination,
        NothingSpecial
    };

    class Buffer
    {
    public:
        Buffer(BufferType type, BufferPurpose purpose, const LogicalDevice& device, hardware::MemoryModule resource_memory, std::size_t bytes);
        Buffer(const Buffer& copy) = delete;
        Buffer(Buffer&& move);
        ~Buffer();

        BufferType get_type() const;

        void write(const void* addr, std::size_t bytes);
        void* map_memory();
        void unmap_memory();

        Buffer& operator=(const Buffer& rhs) = delete;
        Buffer& operator=(Buffer&& rhs);

        VkBuffer native() const;
    private:
        VkBuffer buffer;
        VkDeviceMemory memory;
        const LogicalDevice* device;
        BufferType type;
    };
}

#endif // TZ_VULKAN
#endif //TOPAZ_GL_VK_BUFFER_HPP