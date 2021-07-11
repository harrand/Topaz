#ifndef TOPAZ_GL_VK_PRESENT_HPP
#define TOPAZ_GL_VK_PRESENT_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/semaphore.hpp"
#include "gl/impl/backend/vk/swapchain.hpp"
#include "gl/impl/backend/vk/hardware/queue.hpp"
#include <utility>

namespace tz::gl::vk
{
    class Present
    {
    public:
        Present(const Swapchain& swapchain, std::uint32_t swapchain_image_index, SemaphoreRefs semaphores);
        void operator()(const hardware::Queue& queue) const;
    private:
        void update();
        VkSwapchainKHR swapchain_native;
        std::vector<VkSemaphore> signal_sem_natives;
        VkPresentInfoKHR present;
        std::uint32_t image_index;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PRESENT_HPP