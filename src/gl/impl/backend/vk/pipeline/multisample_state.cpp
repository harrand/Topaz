#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/multisample_state.hpp"

namespace tz::gl::vk::pipeline
{
	MultisampleState::MultisampleState():
	create()
	{
		this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		this->create.sampleShadingEnable = VK_FALSE;
		this->create.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		this->create.minSampleShading = 1.0f;
		this->create.pSampleMask = nullptr;
		this->create.alphaToCoverageEnable = VK_FALSE;
		this->create.alphaToOneEnable = VK_FALSE;
	}

	VkPipelineMultisampleStateCreateInfo MultisampleState::native() const
	{
		return this->create;
	}
}

#endif // TZ_VULKAN