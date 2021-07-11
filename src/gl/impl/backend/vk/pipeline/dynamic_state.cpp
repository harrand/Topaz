#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/dynamic_state.hpp"

namespace tz::gl::vk::pipeline
{
    DynamicState::DynamicState(DynamicStateTypeField state_enables):
    DynamicState()
    {
        this->states_enabled = state_enables;
        auto num_states = this->states_enabled.count();
        this->create.dynamicStateCount = num_states;
        this->state_natives.reserve(num_states);
        for(DynamicStateType type : this->states_enabled)
        {
            this->state_natives.push_back(static_cast<VkDynamicState>(type));
        }
        this->create.pDynamicStates = this->state_natives.data();
    }

    DynamicState DynamicState::None()
    {
        return {};
    }

    VkPipelineDynamicStateCreateInfo DynamicState::native() const
    {
        return this->create;
    }

    DynamicState::DynamicState():
    states_enabled(),
    state_natives(),
    create()
    {
        this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        this->create.pNext = nullptr;
        this->create.pDynamicStates = nullptr;
        this->create.dynamicStateCount = 0;
    }
}

#endif // TZ_VULKAN