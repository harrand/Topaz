#include "gl/impl/backend/vk2/command.hpp"

void empty_command_pool()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

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
	linfo.surface = &get_window_surface();

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

int main()
{
	tz::GameInfo game{"vk_command_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		empty_command_pool();
		basic_command_buffers();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
