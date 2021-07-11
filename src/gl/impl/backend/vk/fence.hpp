#ifndef TOPAZ_GL_VK_FENCE_HPP
#define TOPAZ_GL_VK_FENCE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"

namespace tz::gl::vk
{
    class Fence
    {
    public:
        Fence(const LogicalDevice& device);
        Fence(const Fence& copy) = delete;
        Fence(Fence&& move);
        ~Fence();

        Fence& operator=(const Fence& rhs) = delete;
        Fence& operator=(Fence&& rhs);

        VkFence native() const;
        void wait_for() const;
        void signal() const;
        void wait_then_signal() const;
    private:
        VkFence fence;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_FENCE_HPP