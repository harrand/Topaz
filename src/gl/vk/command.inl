#if TZ_VULKAN
#include "core/assert.hpp"

namespace tz::gl::vk
{
    template<typename... Args>
    std::size_t CommandPool::with(Args&&... args)
    {
        this->buffers.push_back({*this, std::forward<Args>(args)...});
        CommandBuffer& buf = this->buffers.back();
        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = this->command_pool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;

        auto res = vkAllocateCommandBuffers(this->device->native(), &info, &buf.command_buffer);
        tz_assert(res == VK_SUCCESS, "Failed to allocate command buffer");
        return this->buffers.size() - 1;
    }
}

#endif // TZ_VULKAN