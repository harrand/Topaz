#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

void extensions_coherent()
{
	// All of these use the default VulkanInstance.
	using namespace tz::gl::vk2;
	PhysicalDeviceList devices = get_all_devices();
	tz_assert(!devices.empty(), "No physical devices available");

	PhysicalDevice pdev = devices.front();
	// Firstly try to create a LogicalDevice with no extensions/features.
	LogicalDevice ldev1{LogicalDeviceInfo{.physical_device = pdev}};
	// Now with one extension, if there are any.
	if(!pdev.get_supported_extensions().empty())
	{
		DeviceExtensionList exts;
		exts |= pdev.get_supported_extensions().front();
		LogicalDevice ldev2{LogicalDeviceInfo{.physical_device = pdev, .extensions = exts}};
	}
	// Now with one extension and one feature (once again, if there are any)
	if(!pdev.get_supported_extensions().empty() && !pdev.get_supported_features().empty())
	{
		DeviceExtensionList exts;
		exts |= pdev.get_supported_extensions().front();
		PhysicalDeviceFeatureField features;
		features |= pdev.get_supported_features().front();
		LogicalDevice ldev2{LogicalDeviceInfo{.physical_device = pdev, .extensions = exts, .features = features}};
	}
	// And now simply with all features/extensions enabled.
	{
		LogicalDevice ldev3{LogicalDeviceInfo{.physical_device = pdev, .extensions = pdev.get_supported_extensions(), .features = pdev.get_supported_features()}};
	}
}

void custom_instance_and_window_surface(const tz::GameInfo& game)
{
	using namespace tz::gl::vk2;
	// Now try with a custom WindowSurface.
	{
		VulkanInfo window_info{game};
		VulkanInstance window_inst{window_info, tz::ApplicationType::HiddenWindowApplication};
		PhysicalDeviceList devices = get_all_devices(window_inst);	
		PhysicalDevice local_pdev = devices.front();
		WindowSurface window_surf{window_inst, tz::window()};

		LogicalDevice ldev4{LogicalDeviceInfo{.physical_device = local_pdev, .surface = &window_surf}};
	}
}

int main()
{
	tz::GameInfo game{"vk_logical_device_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		extensions_coherent();
		custom_instance_and_window_surface(game);
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}