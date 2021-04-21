#if TZ_VULKAN
#include "gl/vk/pipeline/viewport.hpp"

namespace tz::gl::vk::pipeline
{
    ViewportState::ViewportState(const Swapchain& swapchain):
    create(),
    viewport(),
    scissor()
    {
        this->viewport.x = 0.0f;
        this->viewport.y = 0.0f;
        this->viewport.width = swapchain.get_width();
        this->viewport.height = swapchain.get_height();
        this->viewport.minDepth = 0.0f;
        this->viewport.maxDepth = 1.0f;

        this->scissor.offset = {0, 0};
        this->scissor.extent = swapchain.native_extent();

        create.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        create.viewportCount = 1;
        create.pViewports = &this->viewport;

        create.scissorCount = 1;
        create.pScissors = &this->scissor;
    }

    VkPipelineViewportStateCreateInfo ViewportState::native() const
    {
        return this->create;
    }
}

#endif // TZ_VULKAN