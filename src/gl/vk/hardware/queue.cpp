#if TZ_VULKAN
#include "gl/vk/hardware/queue.hpp"
#include "gl/vk/impl/setup/logical_device.hpp"

namespace tz::gl::vk::hardware
{
    Queue::Queue(const LogicalDevice& device, const DeviceQueueFamily& queue_family, std::uint32_t queue_index):
    queue(VK_NULL_HANDLE)
    {
        vkGetDeviceQueue(device.native(), queue_family.index, queue_index, &this->queue);
    }

    VkQueue Queue::native() const
    {
        return this->queue;
    }
}

#endif // TZ_VULKAN