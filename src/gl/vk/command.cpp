#if TZ_VULKAN

#include "gl/vk/command.hpp"

namespace tz::gl::vk
{
    void CommandBuffer::begin_recording()
    {
        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.pInheritanceInfo = nullptr;
        begin.flags = 0;

        auto res = vkBeginCommandBuffer(this->command_buffer, &begin);
        tz_assert(res == VK_SUCCESS, "Failed to begin recording command buffer");
    }

    void CommandBuffer::begin_recording(OneTimeUseTag onetime_use)
    {
        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.pInheritanceInfo = nullptr;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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

    void CommandBuffer::copy(const Buffer& source, Buffer& destination, std::size_t copy_bytes_length)
    {
        VkBufferCopy cpy{};
        cpy.dstOffset = 0;
        cpy.srcOffset = 0;
        cpy.size = copy_bytes_length;

        vkCmdCopyBuffer(this->command_buffer, source.native(), destination.native(), 1, &cpy);
    }

    void CommandBuffer::bind(const Buffer& buf) const
    {
        auto buf_native = buf.native();
        VkDeviceSize offsets[] = {0};
        switch(buf.get_type())
        {
            case BufferType::Vertex:
                vkCmdBindVertexBuffers(this->command_buffer, 0, 1, &buf_native, offsets);
            break;
            case BufferType::Index:
                vkCmdBindIndexBuffer(this->command_buffer, buf_native, 0, VK_INDEX_TYPE_UINT16);
            break;
            default:
                tz_error("Attempting to bind buffer, but its BufferType is unsupported");
            break;
        }
    }

    void CommandBuffer::draw(std::uint32_t vert_count, std::uint32_t inst_count, std::uint32_t first_index, std::uint32_t first_instance)
    {
        vkCmdDraw(this->command_buffer, vert_count, inst_count, first_index, first_instance);
    }

    void CommandBuffer::draw_indexed(std::uint32_t index_count, std::uint32_t inst_count, std::uint32_t first_index, std::uint32_t vertex_offset, std::uint32_t first_instance)
    {
        vkCmdDrawIndexed(this->command_buffer, index_count, inst_count, first_index, vertex_offset, first_instance);
    }

    void CommandBuffer::reset()
    {
        vkResetCommandBuffer(this->command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
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
        tz_assert(res == VK_SUCCESS, "Failed to create command pool");
    }

    CommandPool::CommandPool(const LogicalDevice& device, const hardware::DeviceQueueFamily& queue_family, RecycleableBufferTag recycleable):
    command_pool(VK_NULL_HANDLE),
    device(&device)
    {
        VkCommandPoolCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create.queueFamilyIndex = queue_family.index;
        create.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto res = vkCreateCommandPool(this->device->native(), &create, nullptr, &this->command_pool);
        tz_assert(res == VK_SUCCESS, "Failed to create command pool");
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

    void CommandPool::clear()
    {
        auto buffer_natives = this->get_buffer_natives();
        vkFreeCommandBuffers(this->device->native(), this->command_pool, buffer_natives.size(), buffer_natives.data());
        this->buffers.clear();
    }

    CommandBuffer& CommandPool::operator[](std::size_t idx)
    {
        return this->buffers[idx];
    }

    const CommandBuffer& CommandPool::operator[](std::size_t idx) const
    {
        return this->buffers[idx];
    }

    std::vector<VkCommandBuffer> CommandPool::get_buffer_natives() const
    {
        std::vector<VkCommandBuffer> buffer_natives;
        for(std::size_t i = 0; i < this->buffers.size(); i++)
        {
            buffer_natives.push_back(this->buffers[i].native());
        }
        return buffer_natives;
    }
}

#endif