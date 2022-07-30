#include "tz/core/tz.hpp"
#include "tz/gl/impl/backend/vk2/fence.hpp"

void basic_fence_operations()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		FenceInfo finfo_on
		{
			.device = &ldev,
			.initially_signalled = true
		};

		FenceInfo finfo_off
		{
			.device = &ldev,
			.initially_signalled = false
		};

		Fence f_on{finfo_on};
		Fence f_off{finfo_off};
		tz_assert(f_on.is_signalled(), "Fence wrongly considered unsignalled.");
		tz_assert(!f_off.is_signalled(), "Fence wrongly considered signalled.");
	}
}

int main()
{
	tz::initialise
	({
		.name = "vk_fence_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		basic_fence_operations();
	}
	tz::terminate();
}
