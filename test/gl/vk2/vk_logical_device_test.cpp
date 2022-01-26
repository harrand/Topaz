#include "core/tz.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"
#include <concepts>

void extensions_coherent()
{
	// All of these use the default VulkanInstance.
	using namespace tz::gl::vk2;
	PhysicalDeviceList devices = get_all_devices();
	tz_assert(!devices.empty(), "No physical devices available");

	PhysicalDevice pdev = devices.front();
	// Firstly try to create a LogicalDevice with no extensions/features.
	{
		LogicalDevice ldev1{LogicalDeviceInfo{.physical_device = pdev}};
	}
	// Now with one extension, if there are any.
	if(!pdev.get_supported_extensions().empty())
	{
		DeviceExtensionList exts;
		exts |= pdev.get_supported_extensions().front();
		LogicalDevice ldev2{LogicalDeviceInfo{.physical_device = pdev, .extensions = exts}};
	}
	// Now with one extension and one feature (once again, if there are any)
	auto pdev_features = pdev.get_supported_features();
	if(!pdev.get_supported_extensions().empty() && !pdev_features.empty())
	{
		DeviceExtensionList exts;
		exts |= pdev.get_supported_extensions().front();
		DeviceFeatureField features;
		features |= pdev_features.front();
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
		VulkanInstance window_inst
		{{
			.game_info = game,
			.app_type = tz::ApplicationType::HiddenWindowApplication,
			.window = &tz::window()
		}};
		PhysicalDeviceList devices = get_all_devices(window_inst);	
		PhysicalDevice local_pdev = devices.front();
		WindowSurface window_surf{window_inst, tz::window()};

		LogicalDevice ldev4{LogicalDeviceInfo{.physical_device = local_pdev}};
	}
}

void semantics()
{
	using namespace tz::gl::vk2;	
	// Ensure constexpr stuff
	static_assert(!tz::copyable<LogicalDevice>, "LogicalDevice is wrongly copyable");
	static_assert(tz::moveable<LogicalDevice>, "LogicalDevice is wrongly not moveable");
	// Ensure move doesn't assert/crash:
	// We will need a valid PhysicalDevice.
	tz_assert(!get_all_devices().empty(), "No valid PhysicalDevices");
	LogicalDeviceInfo dummy;
	dummy.physical_device = get_all_devices().front();
	{
		LogicalDevice l1{dummy};
		LogicalDevice l2{std::move(l1)}; // l1 dies
		LogicalDevice l3 = LogicalDevice::null();
		l3 = std::move(l2); // l2 dies
	}
}

void simulataneous_logical_devices()
{
	using namespace tz::gl::vk2;
	PhysicalDeviceList pdevs = get_all_devices();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdevs.front();
	
	{
		LogicalDevice l1{linfo};
		LogicalDevice l2{linfo};
	}
}

void headless_logical_device(tz::GameInfo game)
{
	using namespace tz::gl::vk2;
	VulkanInstance vinst
	{{
		.game_info = game,
		.app_type = tz::ApplicationType::Headless,
	}};
	{
		PhysicalDevice pdev = get_all_devices(vinst).front();
		LogicalDevice ldev
		{{
			.physical_device = pdev,
			.extensions = {},
			.features = {},
		}};
	}
}

int main()
{
	tz::GameInfo game{"vk_logical_device_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		extensions_coherent();
		custom_instance_and_window_surface(game);
		semantics();
		simulataneous_logical_devices();
		headless_logical_device(game);
	}
	tz::terminate();
}
