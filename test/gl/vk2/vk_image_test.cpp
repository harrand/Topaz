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

int main()
{
	tz::GameInfo game{"vk_image_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	{
		basic_images();
		image_layout_transition();
	}
	tz::terminate();
}
