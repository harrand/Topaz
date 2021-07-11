#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/viewport.hpp"

namespace tz::gl::vk::pipeline
{
    ViewportState::ViewportState(const Swapchain& swapchain, bool use_opengl_coordinate_system):
    ViewportState(swapchain.get_width(), swapchain.get_height(), swapchain.native_extent(), use_opengl_coordinate_system)
    {}

    ViewportState::ViewportState(const Image& image, bool use_opengl_coordinate_system):
    ViewportState(image.get_width(), image.get_height(), VkExtent2D{.width = image.get_width(), .height = image.get_height()}, use_opengl_coordinate_system)
    {}

    VkPipelineViewportStateCreateInfo ViewportState::native() const
    {
        return this->create;
    }

    ViewportState::ViewportState(float width, float height, VkExtent2D extent, bool use_opengl_coordinate_system):
    create(),
    viewport(),
    scissor()
    {
        this->viewport.x = 0.0f;
        this->viewport.y = 0.0f;
        this->viewport.width = width;
        this->viewport.height = height;
        if(use_opengl_coordinate_system)
        {
            this->viewport.height *= -1;
            this->viewport.y += height;
        }
        this->viewport.minDepth = 0.0f;
        this->viewport.maxDepth = 1.0f;

        this->scissor.offset = {0, 0};
        this->scissor.extent = extent;

        create.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        create.viewportCount = 1;
        create.pViewports = &this->viewport;

        create.scissorCount = 1;
        create.pScissors = &this->scissor;
    }
}

#endif // TZ_VULKAN