#if TZ_VULKAN
#include "core/assert.hpp"

namespace tz::gl::vk
{
    template<typename... Args>
    std::size_t CommandPool::with(std::size_t count, Args&&... args)
    {
        auto index = this->buffers.size();
        for(std::size_t i = 0; i < count; i++)
        {
            this->buffers.emplace_back(*this, std::forward<Args>(args)...);
        }
        std::vector<VkCommandBuffer> buffer_natives;
        for(std::size_t i = 0; i < this->buffers.size(); i++)
        {
            buffer_natives.push_back(this->buffers[i].native());
        }
        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = this->command_pool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = count;

        auto res = vkAllocateCommandBuffers(this->device->native(), &info, buffer_natives.data());
        for(std::size_t i = 0; i < this->buffers.size(); i++)
        {
            this->buffers[i].command_buffer = buffer_natives[i];
        }
        tz_assert(res == VK_SUCCESS, "Failed to allocate command buffer");
        return index;
    }
}

#endif // TZ_VULKAN