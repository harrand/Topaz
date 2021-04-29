#if TZ_VULKAN
#include "gl/vk/buffer.hpp"
#include <cstring>

namespace tz::gl::vk
{
    Buffer::Buffer(BufferType type, const LogicalDevice& device, std::size_t size_bytes):
    buffer(VK_NULL_HANDLE),
    memory(VK_NULL_HANDLE),
    device(&device)
    {
        VkBufferCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create.size = static_cast<VkDeviceSize>(size_bytes);
        switch(type)
        {
            case BufferType::Vertex:
                create.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
            default:
                tz_error("Unrecognised BufferType");
            break;
        }
        create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto res = vkCreateBuffer(this->device->native(), &create, nullptr, &this->buffer);
        tz_assert(res == VK_SUCCESS, "Failed to create buffer");

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(this->device->native(), this->buffer, &mem_reqs);
        
        const hardware::Device& phys_dev = *this->device->get_queue_family().dev;
        hardware::MemoryProperties mem_props = phys_dev.get_memory_properties();

        // TODO: Not something this stupid
        hardware::MemoryModule mod = mem_props.unsafe_get_some_module_matching({hardware::MemoryType::HostVisible, hardware::MemoryType::HostCoherent});
        VkMemoryAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = mem_reqs.size;
        alloc.memoryTypeIndex = mod.index;

        res = vkAllocateMemory(this->device->native(), &alloc, nullptr, &this->memory);
        tz_assert(res == VK_SUCCESS, "Failed to allocate device memory for buffer");
        vkBindBufferMemory(this->device->native(), this->buffer, this->memory, 0);
    }

    Buffer::Buffer(Buffer&& move):
    buffer(VK_NULL_HANDLE),
    memory(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    Buffer::~Buffer()
    {
        if(this->buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(this->device->native(), this->buffer, nullptr);
            this->buffer = VK_NULL_HANDLE;
        }
        if(this->memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(this->device->native(), this->memory, nullptr);
            this->memory = VK_NULL_HANDLE;
        }
    }

    void Buffer::write(const void* addr, std::size_t bytes)
    {
        void* data;
        vkMapMemory(this->device->native(), this->memory, 0, bytes, 0, &data);
        std::memcpy(data, addr, bytes);
        vkUnmapMemory(this->device->native(), this->memory);
    }

    Buffer& Buffer::operator=(Buffer&& rhs)
    {
        std::swap(this->buffer, rhs.buffer);
        std::swap(this->memory, rhs.memory);
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkBuffer Buffer::native() const
    {
        return this->buffer;
    }
}

#endif // TZ_VULKAN