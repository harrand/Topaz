#if TZ_VULKAN
#include "gl/vk/pipeline/rasteriser_state.hpp"

namespace tz::gl::vk::pipeline
{
    RasteriserState::RasteriserState
    (
        bool clamp_those_outside_view_frustum,
        bool discard_everything,
        PolygonMode mode,
        float line_width,
        CullingStrategy strategy,
        WindingOrder winding_order,
        bool depth_bias_enabled
    ):
    create()
    {
        create.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        create.depthClampEnable = clamp_those_outside_view_frustum ? VK_TRUE : VK_FALSE;
        create.rasterizerDiscardEnable = discard_everything ? VK_TRUE : VK_FALSE;
        create.polygonMode = static_cast<VkPolygonMode>(mode);
        create.lineWidth = line_width;
        create.cullMode = static_cast<VkCullModeFlags>(strategy);
        create.frontFace = static_cast<VkFrontFace>(winding_order);
        create.depthBiasEnable = depth_bias_enabled ? VK_TRUE : VK_FALSE;

        // These are optional and will be useful for shadow-mapping
        create.depthBiasConstantFactor = 0.0f;
        create.depthBiasClamp = 0.0f;
        create.depthBiasSlopeFactor = 0.0f;
    }

    VkPipelineRasterizationStateCreateInfo RasteriserState::native() const
    {
        return this->create;
    }
}


#endif // TZ_VULKAN