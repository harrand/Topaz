#ifndef TOPAZ_GL_VK_HARDWARE_QUEUE_HPP
#define TOPAZ_GL_VK_HARDWARE_QUEUE_HPP
#if TZ_VULKAN
#include "gl/vk/impl/hardware/queue_family.hpp"
#include <cstdint>

namespace tz::gl::vk
{
    class LogicalDevice;
    namespace hardware
    {
        class Queue
        {
        public:
            Queue(const LogicalDevice& device, const DeviceQueueFamily& queue_family, std::uint32_t queue_index);
            VkQueue native() const;
        private:
            VkQueue queue;
        };
    }
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_HARDWARE_QUEUE_HPP