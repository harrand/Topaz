#if TZ_VULKAN

#include "gl/vk/command.hpp"

namespace tz::gl::vk
{
    void CommandBuffer::begin_recording()
    {
        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.pInheritanceInfo = nullptr;

        auto res = vkBeginCommandBuffer(this->command_buffer, &begin);
        tz_assert(res == VK_SUCCESS, "Failed to begin recording command buffer");
    }

    void CommandBuffer::end_recording()
    {
        auto res = vkEndCommandBuffer(this->command_buffer);
        tz_assert(res == VK_SUCCESS, "Failed to end recording command buffer");
    }

    VkCommandBuffer CommandBuffer::native() const
    {
        return this->command_buffer;
    }

    void CommandBuffer::draw(std::uint32_t vert_count, std::uint32_t inst_count, std::uint32_t first_index, std::uint32_t first_instance)
    {
        vkCmdDraw(this->command_buffer, vert_count, inst_count, first_index, first_instance);
    }

    CommandPool::CommandPool(const LogicalDevice& device, const hardware::DeviceQueueFamily& queue_family):
    command_pool(VK_NULL_HANDLE),
    device(&device)
    {
        VkCommandPoolCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create.queueFamilyIndex = queue_family.index;
        create.flags = 0;

        auto res = vkCreateCommandPool(this->device->native(), &create, nullptr, &this->command_pool);
    }

    CommandPool::CommandPool(CommandPool&& move):
    command_pool(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    CommandPool::~CommandPool()
    {
        if(this->command_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(this->device->native(), this->command_pool, nullptr);
            this->command_pool = VK_NULL_HANDLE;
        }
    }

    CommandPool& CommandPool::operator=(CommandPool&& rhs)
    {
        std::swap(this->command_pool, rhs.command_pool);
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkCommandPool CommandPool::native() const
    {
        return this->command_pool;
    }

    CommandBuffer& CommandPool::operator[](std::size_t idx)
    {
        return this->buffers[idx];
    }

    const CommandBuffer& CommandPool::operator[](std::size_t idx) const
    {
        return this->buffers[idx];
    }
}

#endif