#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_GRAPHICS_PIPELINE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_GRAPHICS_PIPELINE_HPP
#if TZ_VULKAN

#include "tz/gl/impl/vulkan/detail/shader.hpp"
#include "tz/gl/impl/vulkan/detail/fixed_function.hpp"
#include "tz/gl/impl/vulkan/detail/pipeline_layout.hpp"
#include "tz/gl/impl/vulkan/detail/render_pass.hpp"

namespace tz::gl::vk2
{
	struct PipelineState
	{
		ViewportState viewport;
		VertexInputState vertex_input = {};
		RasteriserState rasteriser = {};
		MultisampleState multisample = {};
		DepthStencilState depth_stencil = {};
		ColourBlendState colour_blend = {};
		DynamicState dynamic = {};
	};
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline
	 * Specifies creation flags for a @ref GraphicsPipeline.
	 */
	struct GraphicsPipelineInfo
	{
		/// Query as to whether the various state objects are valid and not-nullptr.
		bool valid() const;
		/// Query as to whether the LogicalDevice `logical_device` is a valid device. That is, the device is not nullptr nor a null device.
		bool valid_device() const;

		ShaderPipelineData shaders;
		PipelineState state;
		const PipelineLayout* pipeline_layout;
		const RenderPass* render_pass;
		struct DynamicRenderingState
		{
			std::vector<VkFormat> colour_attachment_formats = {};
			VkFormat depth_format = VK_FORMAT_UNDEFINED;
		};
		DynamicRenderingState dynamic_rendering_state;
		const LogicalDevice* device;
	};

	struct ComputePipelineInfo
	{
		ShaderPipelineData shader;
		const PipelineLayout* pipeline_layout;

		const LogicalDevice* device;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline
	 * Represents the Graphics Pipeline.
	 */
	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(const GraphicsPipelineInfo& info);
		GraphicsPipeline(const GraphicsPipeline& copy) = delete;
		GraphicsPipeline(GraphicsPipeline&& move);
		~GraphicsPipeline();

		GraphicsPipeline& operator=(const GraphicsPipeline& rhs) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&& rhs);

		const LogicalDevice& get_device() const;
		const GraphicsPipelineInfo& get_info() const;
		void set_layout(PipelineLayout& layout);

		using NativeType = VkPipeline;
		NativeType native() const;
	private:
		VkPipeline pipeline;
		GraphicsPipelineInfo info;
	};

	class ComputePipeline
	{
	public:
		ComputePipeline(const ComputePipelineInfo& info);
		ComputePipeline(const ComputePipeline& copy) = delete;
		ComputePipeline(ComputePipeline&& move);
		~ComputePipeline();

		ComputePipeline& operator=(const ComputePipeline& rhs) = delete;
		ComputePipeline& operator=(ComputePipeline&& rhs);

		const LogicalDevice& get_device() const;
		const ComputePipelineInfo& get_info() const;
		void set_layout(PipelineLayout& layout);

		using NativeType = VkPipeline;
		NativeType native() const;
	private:
		VkPipeline pipeline;
		ComputePipelineInfo info;
	};

	class Pipeline
	{
	public:
		Pipeline(const GraphicsPipelineInfo& graphics_info);
		Pipeline(const ComputePipelineInfo& compute_info);

		PipelineContext get_context() const;
		const LogicalDevice& get_device() const;
		const PipelineLayout& get_layout() const;
		void set_layout(PipelineLayout& layout);

		using NativeType = VkPipeline;
		NativeType native() const;

		static Pipeline null();
		bool is_null() const;
	private:
		Pipeline();

		std::variant<GraphicsPipeline, ComputePipeline, std::monostate> pipeline_variant;
	};

	struct PipelineData
	{
		PipelineData() = default;
		PipelineData(PipelineData&& move)
		{
			*this = std::move(move);
		}
		PipelineData& operator=(PipelineData&& rhs)
		{
			std::swap(this->layout, rhs.layout);
			std::swap(this->data, rhs.data);
			if(!this->layout.is_null())
			{
				this->data.set_layout(this->layout);
			}
			return *this;
		}
		PipelineLayout layout = PipelineLayout::null();
		Pipeline data = Pipeline::null();
	};
}


#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_GRAPHICS_PIPELINE_HPP
