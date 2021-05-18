#if TZ_VULKAN
#include "gl/vk/present.hpp"

namespace tz::gl::vk
{
    Present::Present(const Swapchain& swapchain, std::uint32_t swapchain_image_index, SemaphoreRefs semaphores):
    swapchain_native(swapchain.native()),
    signal_sem_natives(),
    present(),
    image_index(swapchain_image_index)
    {
        this->present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        for(const Semaphore& sem : semaphores)
        {
            signal_sem_natives.push_back(sem.native());
        }

        this->update();
    }

    void Present::operator()(const hardware::Queue& queue) const
    {
        auto res = vkQueuePresentKHR(queue.native(), &this->present);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::Present(hardware::Queue): Failed to present queue");
    }

    void Present::update()
    {
        this->present.waitSemaphoreCount = this->signal_sem_natives.size();
        this->present.pWaitSemaphores = this->signal_sem_natives.data();
        this->present.swapchainCount = 1;
        this->present.pSwapchains = &this->swapchain_native;
        this->present.pImageIndices = &this->image_index;
        this->present.pResults = nullptr;
    }
}
#endif // TZ_VULKAN