#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include <cstdint>

namespace tz::gl::vk2
{
	class LogicalDevice;
	namespace hardware
	{
		struct QueueInfo
		{
			const LogicalDevice* dev;
			std::uint32_t queue_family_idx;
			std::uint32_t queue_idx;
		};

		class Queue
		{
		public:
			Queue(QueueInfo info);

			const QueueInfo& get_info() const;
			const LogicalDevice& get_device() const;

			using NativeType = VkQueue;
			NativeType native() const;
		private:
			VkQueue queue;
			QueueInfo info;
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
