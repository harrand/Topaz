#include "gl/impl/backend/vk2/fence.hpp"

void basic_fence_operations()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

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
	tz::GameInfo game{"vk_fence_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		basic_fence_operations();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
