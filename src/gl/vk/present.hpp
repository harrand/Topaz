#ifndef TOPAZ_GL_VK_PRESENT_HPP
#define TOPAZ_GL_VK_PRESENT_HPP
#if TZ_VULKAN
#include "gl/vk/semaphore.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/hardware/queue.hpp"
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