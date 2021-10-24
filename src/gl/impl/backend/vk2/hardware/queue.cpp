#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/queue.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2::hardware
{
	Queue::Queue(QueueInfo info):
	queue(VK_NULL_HANDLE),
	info(info)
	{
		tz_assert(info.dev != nullptr, "QueueInfo's LogicalDevice was nullptr");
		vkGetDeviceQueue(info.dev->native(), this->info.queue_family_idx, this->info.queue_idx, &this->queue);
	}
}

#endif // TZ_VULKAN
