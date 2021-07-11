#ifndef TOPAZ_GL_VK_PIPELINE_DYNAMIC_STATE_HPP
#define TOPAZ_GL_VK_PIPELINE_DYNAMIC_STATE_HPP
#if TZ_VULKAN
#include "core/containers/enum_field.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk::pipeline
{
    enum class DynamicStateType
    {
        ViewportDimensions = VK_DYNAMIC_STATE_VIEWPORT,
        ScissorDimensions = VK_DYNAMIC_STATE_SCISSOR,
        LineWidth = VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    using DynamicStateTypeField = tz::EnumField<DynamicStateType>;

    class DynamicState
    {
    public:
        DynamicState(DynamicStateTypeField state_enables);
        static DynamicState None();
        VkPipelineDynamicStateCreateInfo native() const;
    private:
        DynamicState();
        DynamicStateTypeField states_enabled;
        std::vector<VkDynamicState> state_natives;
        VkPipelineDynamicStateCreateInfo create;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_DYNAMIC_STATE_HPP