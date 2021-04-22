#if TZ_VULKAN
#include "gl/vk/frame_admin.hpp"
#include "gl/vk/present.hpp"

namespace tz::gl::vk
{
    FrameAdmin::FrameAdmin(const LogicalDevice& device, std::size_t frame_depth):
    device(&device),
    frame_depth(frame_depth),
    image_available_semaphores(),
    render_finish_semaphores()
    {
        for(std::size_t i = 0; i < this->frame_depth; i++)
        {
            this->image_available_semaphores.emplace_back(device);
            this->render_finish_semaphores.emplace_back(device);
        }
    }

    void FrameAdmin::render_frame(hardware::Queue queue, const Swapchain& swapchain, const CommandPool& command_pool, WaitStages wait_stages)
    {
        for(std::size_t i = 0; i < this->frame_depth; i++)
        {
            std::uint32_t image_index = swapchain.acquire_next_image_index(this->image_available_semaphores[i]);
            vk::Submit submit{CommandBuffers{command_pool[image_index]}, SemaphoreRefs{this->image_available_semaphores[i]}, wait_stages, SemaphoreRefs{this->render_finish_semaphores[i]}};
            submit(queue);

            vk::Present present{swapchain, image_index, vk::SemaphoreRefs{this->render_finish_semaphores[i]}};
            present(queue);
        }
    }

}

#endif // TZ_VULKAN