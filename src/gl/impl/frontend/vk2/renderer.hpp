#ifndef TOPAZ_GL_IMPL_FRONTEND_VK2_RENDERER_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK2_RENDERER_HPP
#if TZ_VULKAN
#include "gl/impl/frontend/common/renderer.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/backend/vk2/render_pass.hpp"
#include "gl/impl/backend/vk2/framebuffer.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"
#include "gl/impl/backend/vk2/buffer.hpp"
#include "gl/impl/backend/vk2/pipeline_layout.hpp"
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include "gl/impl/backend/vk2/command.hpp"
#include "gl/impl/backend/vk2/fence.hpp"
#include "gl/impl/backend/vk2/semaphore.hpp"

namespace tz::gl
{
	using RendererBuilderVulkan2 = RendererBuilderBase;

	struct RendererBuilderDeviceInfoVulkan2
	{
		/// LogicalDevice used to create VKAPI objects. Most likely comes from a DeviceVulkan.
		vk2::LogicalDevice* device;
		/// List of output images. If the output is a window, this is likely to be swapchain images or an offscreen headless image.
		std::span<vk2::Image> output_images;
		/// Swapchain if there is one.
		vk2::Swapchain* maybe_swapchain;
	};

	namespace detail
	{
		struct IOManagerInfo
		{
			/// LogicalDevice used to create VKAPI objects.
			const vk2::LogicalDevice* device;
			/// List of output images. If the output is a window, this is likely to be swapchain images or an offscreen headless image.
			std::span<vk2::Image> output_images;
			/// Describes the output image(s). We need to know this because we need to figure out whether the RenderPass should provide a presentable image or not.
			RendererOutputType output_type;
		};

		class IOManager
		{
		public:
			IOManager(IOManagerInfo info);
			std::span<const vk2::Framebuffer> get_output_framebuffers() const;
			std::span<vk2::Framebuffer> get_output_framebuffers();
			const vk2::RenderPass& get_render_pass() const;
			tz::Vec2 get_output_dimensions() const;
		private:
			std::vector<vk2::ImageView> output_views;
			vk2::RenderPass render_pass;
			std::vector<vk2::Framebuffer> output_framebuffers;
		};

		struct ResourceManagerInfo
		{
			const vk2::LogicalDevice* device;
			std::span<const IResource* const> buffer_resources;
			std::span<const IResource* const> texture_resources;
			std::size_t frame_in_flight_count;
		};

		class ResourceManager
		{
		public:
			ResourceManager(ResourceManagerInfo info);
			const vk2::DescriptorLayout& get_descriptor_layout() const;

			std::span<vk2::Buffer> get_buffer_components();
			std::span<const vk2::Buffer> get_buffer_components() const;
			std::span<vk2::Image> get_texture_components();
			std::span<const vk2::Image> get_texture_components() const;

			std::span<const vk2::DescriptorSet> get_descriptor_sets() const;
		private:
			std::vector<vk2::Buffer> buffer_components;
			std::vector<vk2::Image> texture_components;
			std::vector<vk2::ImageView> texture_views;
			vk2::Sampler basic_sampler;
			vk2::DescriptorLayout descriptor_layout;
			vk2::DescriptorPool descriptor_pool;
			vk2::DescriptorPool::AllocationResult descriptors;
		};

		struct GraphicsPipelineManagerInfo
		{
			const vk2::DescriptorLayout* descriptor_layout;
			const vk2::Shader& shader_program;
			const vk2::RenderPass& render_pass;
			std::size_t frame_in_flight_count;
			tz::Vec2 viewport_dimensions;
		};

		class GraphicsPipelineManager
		{
		public:
			GraphicsPipelineManager(GraphicsPipelineManagerInfo info);
			const vk2::GraphicsPipeline& get_pipeline() const;

		private:
			vk2::PipelineLayout make_pipeline_layout(const vk2::DescriptorLayout& descriptor_layout, std::size_t frame_in_flight_count);

			vk2::PipelineLayout pipeline_layout;
			vk2::GraphicsPipeline graphics_pipeline;
		};

		struct CommandProcessorInfo
		{
			vk2::LogicalDevice* device;
			std::size_t frame_in_flight_count;
			RendererOutputType output_type;
			std::span<vk2::Framebuffer> output_framebuffers;
		};

		class CommandProcessor
		{
		public:
			CommandProcessor(CommandProcessorInfo info);
			std::span<const vk2::CommandBuffer> get_render_command_buffers() const;
			std::span<vk2::CommandBuffer> get_render_command_buffers();
			void do_scratch_operations(tz::Action<vk2::CommandBufferRecording&> auto record_commands)
			{
				vk2::CommandBuffer scratch_buf = this->commands.buffers.back();
				{
					vk2::CommandBufferRecording record = scratch_buf.record();
					record_commands(record);
				}
				vk2::Fence work_complete_fence
				{{
					.device = &this->command_pool.get_device()
				}};
				this->graphics_queue->submit
				({
					.command_buffers = {&scratch_buf},
					.waits = {},
					.signal_semaphores = {},
					.execution_complete_fence = &work_complete_fence
				});
				work_complete_fence.wait_until_signalled();
			}
			void set_rendering_commands(tz::Action<vk2::CommandBufferRecording&, std::size_t> auto record_commands)
			{
				for(std::size_t i = 0; i < this->get_render_command_buffers().size(); i++)
				{
					vk2::CommandBufferRecording record = this->get_render_command_buffers()[i].record();
					record_commands(record, i);
				}
			}
			void do_render_work(vk2::Swapchain* maybe_swapchain);
		private:
			bool requires_present;
			vk2::hardware::Queue* graphics_queue;
			vk2::CommandPool command_pool;
			vk2::CommandPool::AllocationResult commands;
			std::size_t frame_in_flight_count;

			std::vector<vk2::BinarySemaphore> image_semaphores;
			std::vector<vk2::BinarySemaphore> render_work_semaphores;
			std::vector<vk2::Fence> in_flight_fences;
			std::vector<const vk2::Fence*> images_in_flight;
			std::uint32_t output_image_index = 0;
			std::size_t current_frame = 0;
		};
	}

	class RendererVulkan2 : public RendererBase
	{
	public:
		RendererVulkan2(RendererBuilderVulkan2 builder, RendererBuilderDeviceInfoVulkan2 device_info);
		~RendererVulkan2();
		virtual void set_clear_colour(tz::Vec4 clear_colour) final{}
		virtual IComponent* get_component(ResourceHandle handle) final{return nullptr;}
		
		virtual void render() final;
		virtual void render(RendererDrawList draws) final{}
	private:
		void setup_static_resources();
		void setup_render_commands();

		vk2::LogicalDevice& vk_device;
		detail::IOManager io_manager;
		detail::ResourceManager resource_manager;
		detail::GraphicsPipelineManager graphics_pipeline_manager;
		detail::CommandProcessor command_processor;
		vk2::Swapchain* maybe_swapchain;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK2_RENDERER_HPP
