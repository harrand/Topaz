#if TZ_VULKAN
#include "tz/core/profile.hpp"
#include "tz/gl/impl/vulkan/detail/graphics_pipeline.hpp"
#include <algorithm>

namespace tz::gl::vk2
{
	PipelineCache::PipelineCache(const LogicalDevice& ldev):
	cache(VK_NULL_HANDLE),
	ldev(&ldev)
	{
		VkPipelineCacheCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.initialDataSize = 0,
			.pInitialData = nullptr
		};
		[[maybe_unused]] VkResult res = vkCreatePipelineCache(this->ldev->native(), &create, nullptr, &this->cache);
		tz::assert(res == VK_SUCCESS, "Failed to create pipeline cache. Probably OOM?");
	}

	PipelineCache::PipelineCache(PipelineCache&& move)
	{
		*this = std::move(move);
	}

	PipelineCache::~PipelineCache()
	{
		if(this->ldev != nullptr)
		{
			vkDestroyPipelineCache(this->ldev->native(), this->cache, nullptr);
			this->cache = VK_NULL_HANDLE;
		}
	}

	PipelineCache& PipelineCache::operator=(PipelineCache&& rhs)
	{
		std::swap(this->cache, rhs.cache);
		std::swap(this->ldev, rhs.ldev);
		return *this;
	}

	bool PipelineCache::is_null() const
	{
		return this->cache == VK_NULL_HANDLE;
	}

	PipelineCache PipelineCache::null()
	{
		return {nullptr};
	}

	PipelineCache::NativeType PipelineCache::native() const
	{
		return this->cache;
	}

	PipelineCache::PipelineCache(std::nullptr_t){}

	bool GraphicsPipelineInfo::valid() const
	{
		return !this->shaders.create_infos.empty()
			&& this->pipeline_layout != nullptr
			&& (this->render_pass != nullptr || this->dynamic_rendering_state != vk2::GraphicsPipelineInfo::DynamicRenderingState{})
			&& this->valid_device();
	}

	bool GraphicsPipelineInfo::valid_device() const
	{
		return this->device != nullptr && !this->device->is_null();
	}

	GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info, const PipelineCache& existing_cache):
	pipeline(VK_NULL_HANDLE),
	info(info)
	{
		TZ_PROFZONE("Vulkan Backend - GraphicsPipeline Create", 0xFFAA0000);
		tz::assert(info.valid(), "GraphicsPipelineInfo was invalid. Please submit a bug report.");
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

		const bool has_tessellation = std::any_of(info.shaders.create_infos.begin(), info.shaders.create_infos.end(), [](const VkPipelineShaderStageCreateInfo& shader_create)
		{
			return shader_create.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
			||     shader_create.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		});

		VkPipelineInputAssemblyStateCreateInfo input_assembly_native =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
			.pNext = nullptr,
			.flags = 0,
			.topology = has_tessellation ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : static_cast<VkPrimitiveTopology>(info.topology),
			.primitiveRestartEnable = VK_FALSE
		};

		std::vector<VkDynamicState> dynamic_states_enabled;
		for(const DynamicStateType state : info.state.dynamic.states)
		{
			dynamic_states_enabled.push_back(static_cast<VkDynamicState>(state));
		}

		auto viewport_state_native = info.state.viewport.native();
		auto rasteriser_state_native = info.state.rasteriser.native();
		auto multisample_state_native = info.state.multisample.native();
		auto depth_stencil_state_native = info.state.depth_stencil.native();
		auto colour_blend_state_native = info.state.colour_blend.native();
		auto dynamic_state_native = VkPipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states_enabled.size()),
			.pDynamicStates = dynamic_states_enabled.data()
		};
		auto tess_state = VkPipelineTessellationStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.patchControlPoints = 3
		};
		auto dynamic_rendering_state = VkPipelineRenderingCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.pNext = nullptr,
			.viewMask = 0,
			.colorAttachmentCount = static_cast<std::uint32_t>(info.dynamic_rendering_state.colour_attachment_formats.size()),
			.pColorAttachmentFormats = info.dynamic_rendering_state.colour_attachment_formats.data(),
			.depthAttachmentFormat = info.dynamic_rendering_state.depth_format,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};
		VkRenderPass render_pass_native = VK_NULL_HANDLE;
		void* next = nullptr;
		if(info.render_pass == nullptr)
		{
			tz::assert(info.dynamic_rendering_state.colour_attachment_formats.size(), "Specified no render pass within graphics pipeline creation flags. This means we're using dynamic rendering, but no colour attachment formats were passed. logic error. please submit a bug report.");
			next = &dynamic_rendering_state;
		}
		else
		{
			render_pass_native = info.render_pass->native();
		}
		VkGraphicsPipelineCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = next,
			.flags = 0,
			.stageCount = static_cast<std::uint32_t>(info.shaders.create_infos.length()),
			.pStages = info.shaders.create_infos.data(),
			.pVertexInputState = &vertex_input_state_native,
			.pInputAssemblyState = &input_assembly_native,
			.pTessellationState = &tess_state,
			.pViewportState = &viewport_state_native,
			.pRasterizationState = &rasteriser_state_native,
			.pMultisampleState = &multisample_state_native,
			.pDepthStencilState = &depth_stencil_state_native,
			.pColorBlendState = &colour_blend_state_native,
			.pDynamicState = &dynamic_state_native,
			.layout = info.pipeline_layout->native(),
			.renderPass = render_pass_native,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		VkResult res = vkCreateGraphicsPipelines(this->get_device().native(), existing_cache.native(), 1, &create, nullptr, &this->pipeline);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create GraphicsPipeline because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to create GraphicsPipeline because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_INVALID_SHADER_NV:
				tz::error("Failed to create GraphicsPipeline because one or more shaders failed to compile/link. Please submit a bug report");
			break;
			default:
				tz::error("Failed to create GraphicsPipeline but cannot determine why. Please submit a bug report.");
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
		tz::assert(this->info.device != nullptr, "GraphicsPipelineInfo contained nullptr LogicalDevice. Please submit a bug report.");
		return *this->info.device;
	}

	const GraphicsPipelineInfo& GraphicsPipeline::get_info() const
	{
		return this->info;
	}

	void GraphicsPipeline::set_layout(PipelineLayout& layout)
	{
		this->info.pipeline_layout = &layout;
	}

	GraphicsPipeline::NativeType GraphicsPipeline::native() const
	{
		return this->pipeline;
	}

	ComputePipeline::ComputePipeline(const ComputePipelineInfo& info, const PipelineCache& existing_cache):
	pipeline(VK_NULL_HANDLE),
	info(info)
	{
		TZ_PROFZONE("Vulkan Backend - ComputePipeline Create", 0xFFAA0000);
		VkComputePipelineCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = this->info.shader.create_infos.front(),
			.layout = this->info.pipeline_layout->native(),
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = 0
		};

		VkResult res = vkCreateComputePipelines(this->get_device().native(), existing_cache.native(), 1, &create, nullptr, &this->pipeline);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create ComputePipeline because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to create ComputePipeline because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_INVALID_SHADER_NV:
				tz::error("Failed to create ComputePipeline because one or more shaders failed to compile/link. Please submit a bug report");
			break;
			default:
				tz::error("Failed to create ComputePipeline but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	ComputePipeline::ComputePipeline(ComputePipeline&& move):
	pipeline(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	ComputePipeline::~ComputePipeline()
	{
		if(this->pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(this->get_device().native(), this->pipeline, nullptr);
			this->pipeline = VK_NULL_HANDLE;
		}
	}

	ComputePipeline& ComputePipeline::operator=(ComputePipeline&& rhs)
	{
		std::swap(this->pipeline, rhs.pipeline);
		std::swap(this->info, rhs.info);
		return *this;
	}

	const LogicalDevice& ComputePipeline::get_device() const
	{
		tz::assert(this->info.device != nullptr, "ComputePipelineInfo contained nullptr LogicalDevice. Please submit a bug report.");
		return *this->info.device;
	}

	const ComputePipelineInfo& ComputePipeline::get_info() const
	{
		return this->info;
	}

	void ComputePipeline::set_layout(PipelineLayout& layout)
	{
		this->info.pipeline_layout = &layout;
	}

	ComputePipeline::NativeType ComputePipeline::native() const
	{
		return this->pipeline;
	}

	Pipeline::Pipeline(const GraphicsPipelineInfo& graphics_info, const PipelineCache& cache):
	pipeline_variant(GraphicsPipeline{graphics_info, cache}){}

	Pipeline::Pipeline(const ComputePipelineInfo& compute_info, const PipelineCache& cache):
	pipeline_variant(ComputePipeline{compute_info, cache}){}

	PipelineContext Pipeline::get_context() const
	{
		tz::assert(!std::holds_alternative<std::monostate>(this->pipeline_variant), "Cannot get context of the null pipeline!");
		if(std::holds_alternative<GraphicsPipeline>(this->pipeline_variant))
		{
			return PipelineContext::graphics;
		}
		return PipelineContext::compute;
	}

	const LogicalDevice& Pipeline::get_device() const
	{
		const LogicalDevice* ldev = nullptr;
		std::visit([&ldev](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr(std::is_same_v<T, std::monostate>)
			{
				tz::error("Attempting to get device for a null Pipeline");
			}
			else
			{
				ldev = &arg.get_device();
			}
		}, this->pipeline_variant);
		tz::assert(ldev != nullptr, "Pipeline had no LogicalDevice attached. Please submit a bug report.");
		return *ldev;
	}

	const PipelineLayout& Pipeline::get_layout() const
	{
		const PipelineLayout* lay = nullptr;
		std::visit([&lay](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr(std::is_same_v<T, std::monostate>)
			{
				tz::error("Attempting to get layout for a null Pipeline");
			}
			else
			{
				lay = arg.get_info().pipeline_layout;
			}
		}, this->pipeline_variant);
		tz::assert(lay != nullptr, "Pipeline had no PipelineLayout attached. Please submit a bug report.");
		return *lay;
	}

	void Pipeline::set_layout(PipelineLayout& layout)
	{
		std::visit([&layout](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr(std::is_same_v<T, std::monostate>)
			{
				tz::error("Attempting to set layout for a null Pipeline");
			}
			else
			{
				arg.set_layout(layout);
			}
		}, this->pipeline_variant);
	}

	Pipeline::NativeType Pipeline::native() const
	{
		Pipeline::NativeType nat = VK_NULL_HANDLE;
		std::visit([&nat](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr(std::is_same_v<T, std::monostate>)
			{
				tz::error("Attempting to retrieve native type for a null Pipeline");
			}
			else
			{
				nat = arg.native();
			}
		}, this->pipeline_variant);
		return nat;
	}

	Pipeline Pipeline::null()
	{
		return {};
	}

	bool Pipeline::is_null() const
	{
		return std::holds_alternative<std::monostate>(this->pipeline_variant);
	}

	Pipeline::Pipeline():
	pipeline_variant{std::monostate{}}
	{
	}
}

#endif // TZ_VULKAN
