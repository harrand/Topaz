#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/colour_blend_state.hpp"

namespace tz::gl::vk::pipeline
{
	ColourBlendState::ColourBlendState(std::size_t attached_framebuffer_count, ColourBlendingStrategy colour_blending):
	attachments(),
	create()
	{
		this->attachments.resize(attached_framebuffer_count);
		for(auto& attach : this->attachments)
		{
			attach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
								|   VK_COLOR_COMPONENT_G_BIT
								|   VK_COLOR_COMPONENT_B_BIT
								|   VK_COLOR_COMPONENT_A_BIT;
			switch(colour_blending)
			{
				case ColourBlendingStrategy::None:
				default:
					attach.blendEnable = VK_FALSE;
					attach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					attach.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					attach.colorBlendOp = VK_BLEND_OP_ADD;
					attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					attach.alphaBlendOp = VK_BLEND_OP_ADD;
				break;
				case ColourBlendingStrategy::AlphaBlending:
					attach.blendEnable = VK_TRUE;
					attach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					attach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					attach.colorBlendOp = VK_BLEND_OP_ADD;
					attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					attach.alphaBlendOp = VK_BLEND_OP_ADD;
				break;
			}
		}

		this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		this->create.logicOpEnable = VK_FALSE; // bitwise combination, no thanks
		this->create.logicOp = VK_LOGIC_OP_COPY; // optional (?)
		this->create.attachmentCount = this->attachments.length();
		this->create.pAttachments = this->attachments.data();
		for(std::size_t i = 0; i < 4; i++)
		{
			this->create.blendConstants[i] = 0.0f;
		}
	}

	VkPipelineColorBlendStateCreateInfo ColourBlendState::native() const
	{
		return this->create;
	}
}

#endif // TZ_VULKAN