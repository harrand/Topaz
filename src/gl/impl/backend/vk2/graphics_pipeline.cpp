#if TZ_VULKAN
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include <algorithm>

namespace tz::gl::vk2
{
	bool GraphicsPipelineInfo::valid() const
	{
		return !this->shaders.create_infos.empty()
			&& this->vertex_input_state != nullptr
			&& this->input_assembly != nullptr
			&& this->viewport_state != nullptr
			&& this->rasteriser_state != nullptr
			&& this->multisample_state != nullptr
			/*this->depth_stencil_state is optional*/
			&& this->colour_blend_state != nullptr
			/*this->dynamic_state is optional*/	
			&& this->pipeline_layout != nullptr
			&& this->render_pass != nullptr
			&& this->valid_device();
	}

	bool GraphicsPipelineInfo::valid_device() const
	{
		return this->device != nullptr && !this->device->is_null();
	}

	GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info):
	pipeline(VK_NULL_HANDLE),
	info(info)
	{
		tz_assert(info.valid(), "GraphicsPipelineInfo was invalid. Please submit a bug report.");
		// https://youtu.be/yyLuGNPLRjc
		// Vertex Input State
		tz_assert(info.vertex_input_state != nullptr, "GraphicsPipelineInfo contained nullptr VertexInputState. Please submit a bug report");
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_natives(info.vertex_input_state->bindings.length());
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_natives(info.vertex_input_state->attributes.length());

		std::transform(info.vertex_input_state->bindings.begin(), info.vertex_input_state->bindings.end(), vertex_input_binding_natives.begin(), [](const VertexInputState::Binding& binding)
		{
			return VkVertexInputBindingDescription
			{
				.binding = binding.binding,
				.stride = binding.stride,
				.inputRate = static_cast<VkVertexInputRate>(binding.input_rate)
			};
		});

		std::transform(info.vertex_input_state->attributes.begin(), info.vertex_input_state->attributes.end(), vertex_input_attribute_natives.data(), [](const VertexInputState::Attribute& attribute)
		{
			return VkVertexInputAttributeDescription
			{
				.location = attribute.shader_location,
				.binding = attribute.binding_id,
				.format = static_cast<VkFormat>(attribute.format),
				.offset = attribute.element_offset
			};
		});
		
		VkPipelineVertexInputStateCreateInfo vertex_input_state_native
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_input_binding_natives.size()),
			.pVertexBindingDescriptions = vertex_input_binding_natives.data(),
			.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_input_attribute_natives.size()),
			.pVertexAttributeDescriptions = vertex_input_attribute_natives.data()
		};

		auto input_assembly_native = info.input_assembly->native();
		auto viewport_state_native = info.viewport_state->native();
		auto rasteriser_state_native = info.rasteriser_state->native();
		auto multisample_state_native = info.multisample_state->native();
		auto depth_stencil_state_native = info.depth_stencil_state->native();
		auto colour_blend_state_native = info.colour_blend_state->native();
		DynamicState::NativeType dynamic_state_native;
		DynamicState::NativeType* dynamic_state_native_ptr = nullptr;
		if(info.dynamic_state != nullptr)
		{
			dynamic_state_native = info.dynamic_state->native();
			dynamic_state_native_ptr = &dynamic_state_native;
		}
		VkGraphicsPipelineCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stageCount = static_cast<std::uint32_t>(info.shaders.create_infos.length()),
			.pStages = info.shaders.create_infos.data(),
			.pVertexInputState = &vertex_input_state_native,
			.pInputAssemblyState = &input_assembly_native,
			.pTessellationState = nullptr,
			.pViewportState = &viewport_state_native,
			.pRasterizationState = &rasteriser_state_native,
			.pMultisampleState = &multisample_state_native,
			.pDepthStencilState = &depth_stencil_state_native,
			.pColorBlendState = &colour_blend_state_native,
			.pDynamicState = dynamic_state_native_ptr,
			.layout = info.pipeline_layout->native(),
			.renderPass = info.render_pass->native(),
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		VkResult res = vkCreateGraphicsPipelines(this->get_device().native(), VK_NULL_HANDLE, 1, &create, nullptr, &this->pipeline);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create GraphicsPipeline because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create GraphicsPipeline because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_INVALID_SHADER_NV:
				tz_error("Failed to create GraphicsPipeline because one or more shaders failed to compile/link. Please submit a bug report");
			break;
			default:
				tz_error("Failed to create GraphicsPipeline but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& move):
	pipeline(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		if(this->pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(this->get_device().native(), this->pipeline, nullptr);
			this->pipeline = VK_NULL_HANDLE;
		}
	}

	GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& rhs)
	{
		std::swap(this->pipeline, rhs.pipeline);
		std::swap(this->info, rhs.info);
		return *this;
	}

	const LogicalDevice& GraphicsPipeline::get_device() const
	{
		tz_assert(this->info.device != nullptr, "GraphicsPipelineInfo contained nullptr LogicalDevice. Please submit a bug report.");
		return *this->info.device;
	}

	const GraphicsPipelineInfo& GraphicsPipeline::get_info() const
	{
		return this->info;
	}

	GraphicsPipeline::NativeType GraphicsPipeline::native() const
	{
		return this->pipeline;
	}
}

#endif // TZ_VULKAN
