#ifndef TOPAZ_GL_VK_PIPELINE_MULTISAMPLE_STATE_HPP
#define TOPAZ_GL_VK_PIPELINE_MULTISAMPLE_STATE_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"

namespace tz::gl::vk::pipeline
{
	// TODO: Enable multisampling
	class MultisampleState
	{
	public:
		MultisampleState();
		VkPipelineMultisampleStateCreateInfo native() const;
	private:
		VkPipelineMultisampleStateCreateInfo create;
	};
}

#endif // TZ_VULKAN
#endif //TOPAZ_GL_VK_PIPELINE_MULTISAMPLE_STATE_HPP