#ifndef TOPAZ_GL_VK_PIPELINE_COLOUR_BLEND_STATE_HPP
#define TOPAZ_GL_VK_PIPELINE_COLOUR_BLEND_STATE_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include "core/containers/basic_list.hpp"

namespace tz::gl::vk::pipeline
{
	using ColourBlendAttachmentState = VkPipelineColorBlendAttachmentState;
	using ColourBlendAttachmentStates = tz::BasicList<ColourBlendAttachmentState>;
	
	enum class ColourBlendingStrategy
	{
		None,
		AlphaBlending
	};
	
	class ColourBlendState
	{
	public:
		ColourBlendState(std::size_t attached_framebuffer_count = 1, ColourBlendingStrategy colour_blending = ColourBlendingStrategy::None);
		VkPipelineColorBlendStateCreateInfo native() const;
	private:
		ColourBlendAttachmentStates attachments; // per attached framebuffer
		VkPipelineColorBlendStateCreateInfo create; // global color blending settings
	};
}

#endif // TZ_VULKAN
#endif //TOPAZ_GL_VK_PIPELINE_COLOUR_BLEND_STATE_HPP