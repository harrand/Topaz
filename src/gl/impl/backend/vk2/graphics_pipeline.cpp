#if TZ_VULKAN
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include <algorithm>

namespace tz::gl::vk2
{
	bool GraphicsPipelineInfo::valid() const
	{
		return !this->shaders.create_infos.empty()
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
		TZ_PROFZONE("Vulkan Backend - GraphicsPipeline Create", TZ_PROFCOL_RED);
		tz_assert(info.valid(), "GraphicsPipelineInfo was invalid. Please submit a bug report.");
		// https://youtu.be/yyLuGNPLRjc
		// Vertex Input State
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_natives(info.state.vertex_input.bindings.length());
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_natives(info.state.vertex_input.attributes.length());

		std::transform(info.state.vertex_input.bindings.begin(), info.state.vertex_input.bindings.end(), vertex_input_binding_natives.begin(), [](const VertexInputState::Binding& binding)
		{
			return VkVertexInputBindingDescription
			{
				.binding = binding.binding,
				.stride = binding.stride,
				.inputRate = static_cast<VkVertexInputRate>(binding.input_rate)
			};
		});

		std::transform(info.state.vertex_input.attributes.begin(), info.state.vertex_input.attributes.end(), vertex_input_attribute_natives.data(), [](const VertexInputState::Attribute& attribute)
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

		VkPipelineInputAssemblyStateCreateInfo input_assembly_native =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
			.pNext = nullptr,
			.flags = 0,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		auto viewport_state_native = info.state.viewport.native();
		auto rasteriser_state_native = info.state.rasteriser.native();
		auto multisample_state_native = info.state.multisample.native();
		auto depth_stencil_state_native = info.state.depth_stencil.native();
		auto colour_blend_state_native = info.state.colour_blend.native();
		auto dynamic_state_native = info.state.dynamic.native();
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
			.pDynamicState = &dynamic_state_native,
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
