#ifndef TOPAZ_GL_VK_FRAME_ADMIN_HPP
#define TOPAZ_GL_VK_FRAME_ADMIN_HPP
#if TZ_VULKAN
#include "core/types.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/submit.hpp"
#include <functional>

namespace tz::gl::vk
{
    class FrameAdmin
    {
    public:
        FrameAdmin(const LogicalDevice& device, std::size_t frame_depth);
        void render_frame(hardware::Queue queue, const Swapchain& swapchain, const CommandPool& command_pool, WaitStages wait_stages);
        void set_regeneration_function(tz::Action auto regeneration_function);
        std::size_t get_image_index() const;
    private:        
        const LogicalDevice* device;
        std::size_t frame_depth;
        std::size_t frame_counter;
        std::uint32_t cur_image_index;
        std::vector<Semaphore> image_available_semaphores;
        std::vector<Semaphore> render_finish_semaphores;
        std::vector<Fence> in_flight_fences;
        std::vector<Fence*> images_in_flight;
        std::function<void()> regenerate_function;
    };
}

#include "gl/vk/frame_admin.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_FRAME_ADMIN_HPP