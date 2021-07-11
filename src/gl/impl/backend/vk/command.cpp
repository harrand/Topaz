#if TZ_VULKAN

#include "gl/impl/backend/vk/command.hpp"

namespace tz::gl::vk
{
    CommandBufferRecording::CommandBufferRecording(CommandBufferRecording&& move):
    command_buffer(nullptr)
    {
        *this = std::move(move);
    }

    CommandBufferRecording::~CommandBufferRecording()
    {
        auto res = vkEndCommandBuffer(this->command_buffer->native());
        tz_assert(res == VK_SUCCESS, "Failed to end command buffer recording");
        this->on_recording_end();
    }
    
    CommandBufferRecording& CommandBufferRecording::operator=(CommandBufferRecording&& rhs)
    {
        std::swap(this->command_buffer, rhs.command_buffer);
        std::swap(this->on_recording_end, rhs.on_recording_end);
        return *this;
    }

    void CommandBufferRecording::buffer_copy_buffer(const Buffer& source, Buffer& destination, std::size_t copy_bytes_length)
    {
        tz_assert(!source.is_null(), "Attempted to record a buffer->buffer copy where the source is a null buffer.");
        tz_assert(!destination.is_null(), "Attempted to record a buffer->buffer copy where the destination is a null buffer.");

        VkBufferCopy cpy{};
        cpy.dstOffset = 0;
        cpy.srcOffset = 0;
        cpy.size = copy_bytes_length;

        vkCmdCopyBuffer(this->command_buffer->native(), source.native(), destination.native(), 1, &cpy);
    }

    void CommandBufferRecording::buffer_copy_image(const Buffer& source, Image& destination)
    {
        tz_assert(!source.is_null(), "Attempted to record a buffer->image copy where the source is a null buffer.");
        VkBufferImageCopy cpy{};
        cpy.bufferOffset = 0;
        cpy.bufferRowLength = 0;
        cpy.bufferImageHeight = 0;

        cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cpy.imageSubresource.mipLevel = 0;
        cpy.imageSubresource.baseArrayLayer = 0;
        cpy.imageSubresource.layerCount = 1;

        cpy.imageOffset = {0, 0, 0};
        cpy.imageExtent = {
            destination.get_width(),
            destination.get_height(),
            1
        };

        vkCmdCopyBufferToImage(this->command_buffer->native(), source.native(), destination.native(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
    }

    void CommandBufferRecording::transition_image_layout(Image& image, Image::Layout new_layout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = static_cast<VkImageLayout>(image.get_layout());
        barrier.newLayout = static_cast<VkImageLayout>(new_layout);

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image.native();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        switch(image.get_layout())
        {
            case Image::Layout::Undefined:
                barrier.srcAccessMask = 0;
                source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
            case Image::Layout::TransferDestination:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
            default:
                tz_error("Image Source Layout is NYI");
            break;
        }

        switch(new_layout)
        {
            case Image::Layout::TransferDestination:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
            case Image::Layout::ShaderResource:
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                destination_stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
            default:
                tz_error("Image Destination Layout is NYI");
            break;
        }

        vkCmdPipelineBarrier(this->command_buffer->native(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void CommandBufferRecording::bind(const Buffer& buf)
    {
        tz_assert(!buf.is_null(), "Attempted to record a bind for a null buffer.");
        auto buf_native = buf.native();
        VkDeviceSize offsets[] = {0};
        switch(buf.get_type())
        {
            case BufferType::Vertex:
                vkCmdBindVertexBuffers(this->command_buffer->native(), 0, 1, &buf_native, offsets);
            break;
            case BufferType::Index:
                vkCmdBindIndexBuffer(this->command_buffer->native(), buf_native, 0, VK_INDEX_TYPE_UINT32);
            break;
            default:
                tz_error("Attempting to bind buffer, but its BufferType is unsupported");
            break;
        }
    }

    void CommandBufferRecording::bind(const DescriptorSet& descriptor_set, const pipeline::Layout& layout)
    {
        auto descriptor_set_native = descriptor_set.native();
        vkCmdBindDescriptorSets(this->command_buffer->native(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout.native(), 0, 1, &descriptor_set_native, 0, nullptr);
    }

    void CommandBufferRecording::draw(std::uint32_t vert_count, std::uint32_t inst_count, std::uint32_t first_index, std::uint32_t first_instance)
    {
        vkCmdDraw(this->command_buffer->native(), vert_count, inst_count, first_index, first_instance);
    }

    void CommandBufferRecording::draw_indexed(std::uint32_t index_count, std::uint32_t inst_count, std::uint32_t first_index, std::uint32_t vertex_offset, std::uint32_t first_instance)
    {
        vkCmdDrawIndexed(this->command_buffer->native(), index_count, inst_count, first_index, vertex_offset, first_instance);
    }

    void CommandBufferRecording::draw_indirect(const vk::Buffer& draw_indirect_buffer, std::uint32_t draw_count)
    {
        tz_assert(!draw_indirect_buffer.is_null(), "Attempted to record a draw-indirect, but the draw-indirect-buffer was a null buffer.");
        vkCmdDrawIndexedIndirect(this->command_buffer->native(), draw_indirect_buffer.native(), 0, draw_count, sizeof(VkDrawIndexedIndirectCommand));
    }

    CommandBufferRecording::CommandBufferRecording(const CommandBuffer& buffer, std::function<void()> on_recording_end):
    command_buffer(&buffer),
    on_recording_end(on_recording_end)
    {
        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.pInheritanceInfo = nullptr;
        begin.flags = 0;

        auto res = vkBeginCommandBuffer(this->command_buffer->native(), &begin);
        tz_assert(res == VK_SUCCESS, "Failed to begin command buffer recording");
    }

    CommandBuffer::CommandBuffer([[maybe_unused]] const CommandPool& parent):
    command_buffer(VK_NULL_HANDLE),
    currently_recording(false)
    {
        // Note: this->command_buffer is handled by CommandPool.
    }

    CommandBufferRecording CommandBuffer::record()
    {
        this->notify_recording_begin();
        return {*this, [this](){this->notify_recording_end();}};
    }

    VkCommandBuffer CommandBuffer::native() const
    {
        return this->command_buffer;
    }

    void CommandBuffer::reset()
    {
        tz_assert(!this->currently_recording, "Cannot reset a command buffer that is currently recording");
        vkResetCommandBuffer(this->command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }

    void CommandBuffer::notify_recording_begin()
    {
        tz_assert(!this->currently_recording, "Command buffer double recording detected");
        this->currently_recording = true;
    }

    void CommandBuffer::notify_recording_end()
    {
        tz_assert(this->currently_recording, "Command buffer tried to end recording but wasn't recording");
        this->currently_recording = false;
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

    CommandPool::CommandPool(const LogicalDevice& device, const hardware::DeviceQueueFamily& queue_family, [[maybe_unused]] RecycleableBufferTag recycleable):
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
    device(nullptr),
    buffers()
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
        std::swap(this->buffers, rhs.buffers);
        return *this;
    }

    VkCommandPool CommandPool::native() const
    {
        return this->command_pool;
    }

    bool CommandPool::empty() const
    {
        return this->buffers.empty();
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