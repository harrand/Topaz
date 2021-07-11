#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/input_assembly.hpp"

namespace tz::gl::vk::pipeline
{
    InputAssembly::InputAssembly(PrimitiveTopology topology, bool breakable_strips):
    create()
    {
        this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        this->create.topology = static_cast<VkPrimitiveTopology>(topology);
        this->create.primitiveRestartEnable = breakable_strips ? VK_TRUE : VK_FALSE;
    }

    VkPipelineInputAssemblyStateCreateInfo InputAssembly::native() const
    {
        return this->create;
    }
}

#endif // TZ_VULKAN