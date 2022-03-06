#include "core/vector.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/command.hpp"
#include "gl/impl/backend/vk2/fence.hpp"

void basic_images()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDevice ldev
	{{
		.physical_device = pdev,
	}};

	ImageFormat fmt = format_traits::get_mandatory_colour_attachment_formats().front();
	
	Image basic_img
	{{
		.device = &ldev,
		.format = fmt,
		.dimensions = {2u, 2u},
		.usage = {ImageUsage::ColourAttachment},
		.residency = MemoryResidency::GPU,
		.image_tiling = ImageTiling::Optimal
	}};

	tz_assert(basic_img.get_dimensions() == tz::Vec2ui(2u, 2u), "Image had unexpected dimensions. Expected {%u, %u} but got {%u, %u}", 2u, 2u, basic_img.get_dimensions()[0], basic_img.get_dimensions()[1]);
	tz_assert(basic_img.get_layout() == ImageLayout::Undefined, "Image had unexpected initial layout. Expected ImageLayout::Undefined");
	tz_assert(basic_img.get_format() == fmt, "Image had unexpected format.");
}

void image_layout_transition()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDevice ldev
	{{
		.physical_device = pdev,
	}};
	hardware::Queue* queue = ldev.get_hardware_queue
	({
		.field = {QueueFamilyType::Transfer}
	});

	ImageFormat fmt = format_traits::get_mandatory_colour_attachment_formats().front();
	
	Image basic_img
	{{
		.device = &ldev,
		.format = fmt,
		.dimensions = {2u, 2u},
		.usage = {ImageUsage::TransferDestination},
		.residency = MemoryResidency::GPU,
		.image_tiling = ImageTiling::Optimal
	}};

	CommandPool cpool
	{{
		.queue = queue
	}};
	CommandPool::AllocationResult alloc_res = cpool.allocate_buffers
	({
		.buffer_count = 1
	});

	CommandBuffer& cbuf = alloc_res.buffers.front();
	{
		CommandBufferRecording record = cbuf.record();
		record.transition_image_layout
		({
			.image = &basic_img,
			.target_layout = ImageLayout::TransferDestination,
			.source_access = {AccessFlag::None},
			.destination_access = {AccessFlag::TransferOperationWrite},
			.source_stage = PipelineStage::Top,
			.destination_stage = PipelineStage::TransferCommands,
			.image_aspects = {ImageAspectFlag::Colour}
		});
	}

	/*
	 * - Ensure ImageLayout is undefined (thats what images should always start out as)
	 * - Submit the command buffer which should transition to TransferDestination.
	 * - Ensure ImageLayout is TransferDestination.
	 */
	Fence fence
	{{
		.device = &ldev,
	}};

	tz_assert(basic_img.get_layout() == ImageLayout::Undefined, "Image had unexpected layout pre-transition");

	queue->submit
	({
		.command_buffers = {&cbuf},
		.waits = {},
		.signal_semaphores = {},
		.execution_complete_fence = &fence
	});
	fence.wait_until_signalled();

	tz_assert(basic_img.get_layout() == ImageLayout::TransferDestination, "Image had unexpected layout post-transition");
}

void image_layout_end_render_pass()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDevice ldev
	{{
		.physical_device = pdev,
	}};
	hardware::Queue* queue = ldev.get_hardware_queue
	({
		.field = {QueueFamilyType::Transfer}
	});

	constexpr ImageFormat fmt = ImageFormat::BGRA32;
	
	{
		// We hard-code bgra32 here because we need to know the size of the pixels. As of writing this is a guaranteed format for colour attachments, but I would like to assert on it.
		auto allowed = format_traits::get_mandatory_colour_attachment_formats();
		tz_assert(std::find(allowed.begin(), allowed.end(), fmt) != allowed.end(), "Vulkan spec has changed mandatory colour attachment formats, BGRA32 is no longer guaranteed. This unit test needs to be changed to another similar format.");
	}
	Image basic_img
	{{
		.device = &ldev,
		.format = fmt,
		.dimensions = {2u, 2u},
		.usage = {ImageUsage::ColourAttachment, ImageUsage::TransferDestination},
		.residency = MemoryResidency::GPU,
		.image_tiling = ImageTiling::Optimal
	}};
	ImageView basic_img_view
	{{
		.image = &basic_img,
		.aspect = ImageAspect::Colour
	}};

	Buffer image_data_buffer
	{{
		.device = &ldev,
		.size_bytes = 2 * 2 * 4,
		.usage = {BufferUsage::TransferSource},
		.residency = MemoryResidency::GPU
	}};

	CommandPool cpool
	{{
		.queue = queue
	}};
	CommandPool::AllocationResult alloc_res = cpool.allocate_buffers
	({
		.buffer_count = 1
	});

	RenderPassBuilder rbuilder;
	rbuilder.set_device(ldev);
	rbuilder.with_attachment
	({
		.format = fmt,
		.initial_layout = ImageLayout::Undefined,
		.final_layout = ImageLayout::TransferDestination
	});
	SubpassBuilder sbuilder;
	sbuilder.set_pipeline_context(PipelineContext::Graphics);
	sbuilder.with_colour_attachment
	({
		.attachment_idx = 0,
		.current_layout = ImageLayout::ColourAttachment
	});
	rbuilder.with_subpass(sbuilder.build());
	RenderPass pass = rbuilder.build();

	Framebuffer framebuffer
	{{
		.render_pass = &pass,
		.attachments = {&basic_img_view},
		.dimensions = basic_img.get_dimensions()
	}};

	CommandBuffer& cbuf = alloc_res.buffers.front();
	{
		CommandBufferRecording record = cbuf.record();
		{
			CommandBufferRecording::RenderPassRun run{framebuffer, record};
		}
		// When we do a buffer->image copy, the current layout of the image needs to be known. When a render pass run ends, the image layout has changed. Let's test if it worked!
		record.buffer_copy_image
		({
			.src = &image_data_buffer,
			.dst = &basic_img,
			.image_aspects = {ImageAspectFlag::Colour}
		});
	}

	Fence fence
	{{
		.device = &ldev,
	}};

	tz_assert(basic_img.get_layout() == ImageLayout::Undefined, "Image had unexpected layout post-render-pass-run");

	queue->submit
	({
		.command_buffers = {&cbuf},
		.waits = {},
		.signal_semaphores = {},
		.execution_complete_fence = &fence
	});
	fence.wait_until_signalled();

	tz_assert(basic_img.get_layout() == ImageLayout::TransferDestination, "Image had unexpected layout post-transition");
}

int main()
{
	tz::initialise
	({
		.name = "vk_image_test",
		.app_type = tz::ApplicationType::Headless
	});
	{
		basic_images();
		image_layout_transition();
		image_layout_end_render_pass();
	}
	tz::terminate();
}
