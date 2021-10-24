#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

int main()
{
	tz::GameInfo game{"vk_logical_device_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		using namespace tz::gl::vk2;
		PhysicalDeviceList devices = get_all_devices();
		tz_assert(!devices.empty(), "No physical devices available");

		PhysicalDevice pdev = devices.front();
		// Firstly try to create a LogicalDevice with no extensions/features.
		LogicalDevice ldev1{pdev};
		// Now with one extension, if there are any.
		if(!pdev.get_supported_extensions().empty())
		{
			DeviceExtensionList exts;
			exts |= pdev.get_supported_extensions().front();
			LogicalDevice ldev2{pdev, exts};
		}
		// Now with one extension and one feature (once again, if there are any)
		if(!pdev.get_supported_extensions().empty() && !pdev.get_supported_features().empty())
		{
			DeviceExtensionList exts;
			exts |= pdev.get_supported_extensions().front();
			PhysicalDeviceFeatureField features;
			features |= pdev.get_supported_features().front();
			LogicalDevice ldev2{pdev, exts, features};
		}
		// And now simply with all features/extensions enabled.
		{
			LogicalDevice ldev3{pdev, pdev.get_supported_extensions(), pdev.get_supported_features()};
		}
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
