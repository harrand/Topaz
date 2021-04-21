#ifndef TOPAZ_GL_VK_PIPELINE_RASTERISER_STATE_HPP
#define TOPAZ_GL_VK_PIPELINE_RASTERISER_STATE_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"

namespace tz::gl::vk::pipeline
{
    enum class PolygonMode
    {
        Fill = VK_POLYGON_MODE_FILL,
        Lines = VK_POLYGON_MODE_LINE,
        Points = VK_POLYGON_MODE_POINT
    };

    enum class CullingStrategy
    {
        Back = VK_CULL_MODE_BACK_BIT,
        Front = VK_CULL_MODE_FRONT_BIT,
        Both = VK_CULL_MODE_FRONT_AND_BACK,
        None = VK_CULL_MODE_NONE
    };

    enum class WindingOrder
    {
        Clockwise = VK_FRONT_FACE_CLOCKWISE,
        Anticlockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE
    };

    class RasteriserState
    {
    public:
        RasteriserState
        (
            bool clamp_those_outside_view_frustum = false,
            bool discard_everything = false,
            PolygonMode mode = PolygonMode::Fill,
            float line_width = 1.0f,
            CullingStrategy strategy = CullingStrategy::Back,
            WindingOrder winding_order = WindingOrder::Clockwise,
            bool depth_bias_enabled = false
        );

        VkPipelineRasterizationStateCreateInfo native() const;
    private:
        VkPipelineRasterizationStateCreateInfo create;
    };
}

#endif // TZ_VULKAN
#endif //TOPAZ_GL_VK_PIPELINE_RASTERISER_STATE_HPP