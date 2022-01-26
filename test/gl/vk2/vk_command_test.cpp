#include "gl/impl/backend/vk2/command.hpp"

void empty_command_pool()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		// Give me any graphics queue, no present support needed.
		const hardware::Queue* graphics_queue_ptr = ldev.get_hardware_queue
		({
			.field = {QueueFamilyType::Graphics},
			.present_support = false
		});
		tz_assert(graphics_queue_ptr != nullptr, "LogicalDevice had no graphics queues");
		
		CommandPoolInfo cinfo
		{
			.queue = graphics_queue_ptr
		};

		CommandPool cpool{cinfo};
	}
}

void basic_command_buffers()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		// Give me any graphics queue, no present support needed.
		const hardware::Queue* graphics_queue_ptr = ldev.get_hardware_queue
		({
			.field = {QueueFamilyType::Graphics},
			.present_support = false
		});
		tz_assert(graphics_queue_ptr != nullptr, "LogicalDevice had no graphics queues");
		
		CommandPoolInfo cinfo
		{
			.queue = graphics_queue_ptr
		};

		CommandPool cpool{cinfo};

		CommandPool::Allocation alloc
		{
			.buffer_count = 3
		};
		CommandPool::AllocationResult result = cpool.allocate_buffers(alloc);
		tz_assert(result.success(), "CommandPool failed to perform a basic allocation");
	}

}

void empty_command_buffer_recording()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		// Give me any graphics queue, no present support needed.
		const hardware::Queue* graphics_queue_ptr = ldev.get_hardware_queue
		({
			.field = {QueueFamilyType::Graphics},
			.present_support = false
		});
		tz_assert(graphics_queue_ptr != nullptr, "LogicalDevice had no graphics queues");

		RenderPassBuilder rbuilder;
		rbuilder.set_device(ldev);
		rbuilder.with_subpass
		({
			.context = PipelineContext::Graphics,
		});
		RenderPass pass = rbuilder.build();

		FramebufferInfo finfo
		{
			.render_pass = &pass,
			.dimensions = {1u, 1u}
		};

		Framebuffer fb{finfo};
		
		CommandPoolInfo cinfo
		{
			.queue = graphics_queue_ptr
		};

		CommandPool cpool{cinfo};

		CommandPool::Allocation alloc
		{
			.buffer_count = 2
		};
		CommandPool::AllocationResult result = cpool.allocate_buffers(alloc);
		tz_assert(result.success(), "CommandPool failed to perform a basic allocation");

		CommandBuffer& cbuf1 = result.buffers.front();
		tz_assert(!cbuf1.is_recording(), "Newly-allocated CommandBuffer wrongly considered to be recording.");
		CommandBuffer& cbuf2 = result.buffers.back();
		tz_assert(!cbuf2.is_recording(), "Newly-allocated CommandBuffer wrongly considered to be recording.");
		// Try a recording.
		{
			CommandBufferRecording recording = cbuf1.record();
			tz_assert(cbuf1.is_recording(), "CommandBuffer wrongly considered to not be recording");
		}
		tz_assert(!cbuf1.is_recording(), "CommandBuffer wrongly considered to be recording");
		// Now one where we use a renderpass.
		{
			tz_assert(!cbuf2.is_recording(), "CommandBuffer wrongly considered to be recording");
			CommandBufferRecording recording = cbuf2.record();
			tz_assert(cbuf2.is_recording(), "CommandBuffer wrongly considered to not be recording");
			{
				CommandBufferRecording::RenderPassRun run{fb, recording};
			}
		}
	}
}

int main()
{
	tz::GameInfo game{"vk_command_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	{
		empty_command_pool();
		basic_command_buffers();
		empty_command_buffer_recording();
	}
	tz::terminate();
}
