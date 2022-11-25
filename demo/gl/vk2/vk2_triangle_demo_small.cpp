#include "tz/core/window.hpp"
#include "tz/gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "tz/gl/impl/backend/vk2/logical_device.hpp"
#include "tz/gl/impl/backend/vk2/swapchain.hpp"
#include "tz/gl/impl/backend/vk2/render_pass.hpp"
#include "tz/gl/impl/backend/vk2/graphics_pipeline.hpp"
#include "tz/gl/impl/backend/vk2/framebuffer.hpp"
#include "tz/gl/impl/backend/vk2/command.hpp"
#include "tz/gl/impl/backend/vk2/semaphore.hpp"
#include "tz/gl/impl/backend/vk2/fence.hpp"

#include "vk2_triangle_demo_common.hpp"

// An extremely simple demo which renders a single triangle using screen-space coordinates and fixed colour shading.

int main()
{
	tz::initialise
	({
		.name = "vk2_triangle_demo_small",
		.window = {.flags = {.resizeable = false}}
	});
	{
		// First start with basic state (get a LogicalDevice and Swapchain running)
		using namespace tz::gl::vk2;
		PhysicalDevice pdev = get_all_devices().front();

		LogicalDevice ldev
		{{
			.physical_device = pdev,
			.extensions = {DeviceExtension::Swapchain},
		}};
		PhysicalDeviceSurfaceCapabilityInfo pdev_capability = pdev.get_surface_capabilities();

		ImageFormat swapchain_format = pdev.get_supported_surface_formats().front();

		Swapchain swapchain
		{{

			.device = &ldev,
			.swapchain_image_count_minimum = pdev_capability.min_image_count,
			.image_format = swapchain_format,
			.present_mode = pdev.get_supported_surface_present_modes().front(),
		}};

		// Create RenderPass.
		// Pass consists of 1 attachment (swapchain image, initial layout: Undefined, final layout: Present)
		// 1 subpass (attachment 0: Undefined -> Colour Attachment)
		RenderPassBuilder rbuilder;
		rbuilder.set_device(ldev);
		rbuilder.with_attachment
		({
			.format = swapchain_format,
			.initial_layout = ImageLayout::Undefined,
			.final_layout = ImageLayout::Present
		});
		
		SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(PipelineContext::Graphics);
		sbuilder.with_colour_attachment
		({
			.attachment_idx = 0,
			.current_layout = ImageLayout::ColourAttachment
		});

		rbuilder.with_subpass(sbuilder.build());
		RenderPass render_pass{rbuilder.build()};

		// Now create GraphicsPipeline.
		DescriptorLayoutBuilder lbuilder;
		lbuilder.set_device(ldev);
		DescriptorLayout dlayout{lbuilder.build()};

		PipelineLayout pipeline_layout
		{{
			.descriptor_layouts = {&dlayout},
			.logical_device = &ldev
		}};

		Shader shader
		{{
			.device = &ldev,
			.modules =
			{
				ShaderModuleInfo
				{
					.device = &ldev,
					.type = ShaderType::Vertex,
					.code = read_shader_code("demo/gl/vk2/shaders/triangle_demo.vertex.tzsl.spv")
				},
				
				ShaderModuleInfo
				{
					.device = &ldev,
					.type = ShaderType::Fragment,
					.code = read_shader_code("demo/gl/vk2/shaders/triangle_demo.fragment.tzsl.spv")
				}
			}
		}};

		GraphicsPipeline graphics_pipeline
		{{
			.shaders = shader.native_data(),
			.state = PipelineState
			{
				.viewport = create_basic_viewport({tz::window().get_width(), tz::window().get_height()})

			},
			.pipeline_layout = &pipeline_layout,
			.render_pass = &render_pass,

			.device = &ldev
		}};

		// Create framebuffers from swapchain images.
		std::vector<Framebuffer> swapchain_framebuffers;
		swapchain_framebuffers.reserve(swapchain.get_image_views().size());
		for(ImageView& swapchain_image_view : swapchain.get_image_views())
		{
			swapchain_framebuffers.emplace_back
			(FramebufferInfo{
				.render_pass = &render_pass,
				.attachments = {&swapchain_image_view},
				.dimensions = {swapchain_image_view.get_image().get_dimensions()}
			});
		}

		// Create and record command buffers. One command buffer per swapchain image
		// Retrieve a hardware queue which we can use.
		hardware::Queue* queue = ldev.get_hardware_queue
		({
			.field = {QueueFamilyType::Graphics},
			.present_support = true
		});

		CommandPool cpool
		{{
			.queue = queue
		}};
		CommandPool::AllocationResult result = cpool.allocate_buffers
		({
			.buffer_count = static_cast<std::uint32_t>(swapchain.get_images().size())
		});
		hdk::assert(result.success(), "CommandPool allocation failed unexpectedly");
		hdk::assert(result.buffers.length() == swapchain.get_images().size(), "Did not allocate correct number of CommandBuffers");

		for(std::size_t i = 0; i < result.buffers.length(); i++)
		{
			CommandBuffer& command_buf = result.buffers[i];
			CommandBufferRecording recording = command_buf.record();
			{
				CommandBufferRecording::RenderPassRun run{swapchain_framebuffers[i], recording};
				recording.bind_pipeline
				({
					.pipeline = &graphics_pipeline,
					.pipeline_context = PipelineContext::Graphics
				});
				recording.draw
				({
					.vertex_count = 3,
					.instance_count = 1,
					.first_vertex = 0,
					.first_instance = 0
				});
			}
		}

		// Now create sync objects for frame administration.
		std::vector<BinarySemaphore> image_available_semaphores;
		std::vector<BinarySemaphore> render_finished_semaphores;
		std::vector<Fence> in_flight_fences;
		std::vector<const Fence*> images_in_flight(max_frames_in_flight, nullptr); 
		for(std::size_t i = 0; i < max_frames_in_flight; i++)
		{
			image_available_semaphores.emplace_back(ldev);
			render_finished_semaphores.emplace_back(ldev);
			in_flight_fences.emplace_back(FenceInfo
			{
				.device = &ldev,
				.initially_signalled = true
			});
		}

		// Main game loop

		std::uint32_t swapchain_image_index;
		std::size_t current_frame = 0;

		while(!tz::window().is_close_requested())
		{
			// Frame admin stuff
			in_flight_fences[current_frame].wait_until_signalled();

			swapchain_image_index = swapchain.acquire_image
			({
				.signal_semaphore = &image_available_semaphores[current_frame],
			}).image_index;

			if(images_in_flight[swapchain_image_index] != nullptr)
			{
				images_in_flight[swapchain_image_index]->wait_until_signalled();
			}
			images_in_flight[swapchain_image_index] = &in_flight_fences[swapchain_image_index];

			in_flight_fences[current_frame].unsignal();
			queue->submit
			({
				.command_buffers = {&result.buffers[swapchain_image_index]},
				.waits = 
				{
					hardware::Queue::SubmitInfo::WaitInfo
					{
						.wait_semaphore = &image_available_semaphores[current_frame],
						.wait_stage = PipelineStage::ColourAttachmentOutput
					}
				},
				.signal_semaphores = {&render_finished_semaphores[current_frame]},
				.execution_complete_fence = &in_flight_fences[current_frame]
			});

			hardware::Queue::PresentResult res = queue->present
			({
			 	.wait_semaphores = {&render_finished_semaphores[current_frame]},
				.swapchain = &swapchain,
				.swapchain_image_index = swapchain_image_index
			});
			hdk::assert(res == hardware::Queue::PresentResult::Success || res == hardware::Queue::PresentResult::Success_Suboptimal, "Presentation failed.");

			current_frame = (current_frame + 1) % max_frames_in_flight;

			tz::window().update();
		}
		ldev.wait_until_idle();
	}
	tz::terminate();
}
